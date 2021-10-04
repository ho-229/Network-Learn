/**
 * @author Ho 229
 * @date 2021/10/3
 */

#include "epoll.h"
#include "tcpsocket.h"
#include "sslsocket.h"
#include "httprequest.h"
#include "httpresponse.h"
#include "httpservices.h"
#include "connectionpool.h"

#include <fstream>

ConnectionPool::ConnectionPool() :
    m_epoll(new Epoll)
{
    m_connections.reserve(512);
}

ConnectionPool::~ConnectionPool()
{

}

void ConnectionPool::addConnection(std::shared_ptr<AbstractSocket> socket)
{
    m_connections.insert(Connection(socket->descriptor(), socket));
    m_epoll->addConnection(socket->descriptor());
}

void ConnectionPool::exec(int interval)
{
    m_runnable = true;

    while(m_runnable)
    {
        this->eventsHandler(m_epoll->epoll(interval));

        // Clean up timeout connections
        Socket socket;
        while(m_manager.checkTop(socket))
        {
            const auto it = m_connections.find(socket);
            if(it != m_connections.end())
                this->release(it->second.get());
        }
    }
}

bool ConnectionPool::session(AbstractSocket * const socket)
{
    std::string raw;

    socket->read(raw);

    auto httpRequest = std::make_shared<HttpRequest>(raw);

    if(!httpRequest->isValid())
        return false;

    socket->addTimes();

    std::string response;
    std::shared_ptr<char[]> sendBuf(new char[SOCKET_BUF_SIZE]);

    auto httpResponse = std::make_shared<HttpResponse>();

    m_services->service(httpRequest.get(), httpResponse.get());

    if(!httpRequest->isKeepAlive() || socket->times() > m_maxTimes)
        httpResponse->setRawHeader("Connection", "close");

    httpResponse->toRawData(response);

    if(socket->write(response) <= 0)
        return false;

    // Send file
    if(httpResponse->bodyType() == HttpResponse::File
        && httpRequest->method() == "GET")
    {
        std::ifstream out(httpResponse->filePath(), std::ios::binary);
        sendFile(out, socket);
    }

    return httpRequest->isKeepAlive() && socket->times() <= m_maxTimes;
}

void ConnectionPool::eventsHandler(const EventList &events)
{
    for(const auto& item : events)
    {
        if(item.events == 0)
            continue;

#ifdef _WIN32
        const Socket fd = item.fd;
#else
        const Socket fd = item.data.fd;
#endif

        const auto it = m_connections.find(fd);

        if(it == m_connections.end())
            m_epoll->removeConnection(fd);
        else if(item.events & CLOSE_EVENT || item.events & ERROR_EVENT)
        {
            this->release(it->second.get());
            continue;
        }

        if(it->second->isListening())
        {
            while(true)
            {
                const SocketInfo info =
                    static_cast<TcpSocket *>(it->second.get())->accept();

                if(!AbstractSocket::isValid(info.descriptor))
                    break;

                AbstractSocket *socket = it->second->sslEnable() ?
                    static_cast<AbstractSocket *>(new SslSocket(info)) :
                    static_cast<AbstractSocket *>(new TcpSocket(info));

                socket->setTimer(m_manager.addTimer(socket->descriptor()));
                this->addConnection(std::shared_ptr<AbstractSocket>(socket));

                ConnectEvent event(socket, ConnectEvent::Accpet);
                m_handler(&event);
            }
        }
        else
        {
            if(!this->session(it->second.get()))
                this->release(it->second.get());
            else
            {
                it->second->timer()->deleteLater();
                it->second->setTimer(m_manager.addTimer(it->second->descriptor()));
            }
        }
    }
}

void ConnectionPool::release(const AbstractSocket *socket)
{
    ConnectEvent event(socket, ConnectEvent::Close);
    m_handler(&event);

    socket->timer()->deleteLater();
    m_epoll->removeConnection(socket->descriptor());
    m_connections.erase(socket->descriptor());
}

bool ConnectionPool::sendFile(std::ifstream &stream, AbstractSocket *socket)
{
    if(!stream || !socket)
        return false;

    std::shared_ptr<char[]> sendBuf(new char[SOCKET_BUF_SIZE]());

    while(!stream.eof())
    {
        stream.read(sendBuf.get(), SOCKET_BUF_SIZE);

        if(socket->write(sendBuf.get(), int(stream.gcount())) <= 0)
            return false;
    }

    return true;
}
