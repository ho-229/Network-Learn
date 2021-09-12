/**
 * @author Ho 229
 * @date 2021/7/26
 */

#include "webserver.h"

#include "until.h"
#include "tcpsocket.h"
#include "sslsocket.h"
#include "httpservices.h"

#include <fstream>

#include <signal.h>

#ifdef _WIN32
# include <WinSock2.h>
#else
# include <sys/select.h>
#endif

WebServer::WebServer() :
    m_epoll(new Epoll()),
    m_pool(),
    m_services(new HttpServices())
{
#ifdef _WIN32
    m_isLoaded = TcpSocket::initializatWsa();
#else   // Unix
    signal(SIGPIPE, SIG_IGN);       // Ignore SIGPIPE
#endif
}

WebServer::~WebServer()
{
#ifdef _WIN32
    TcpSocket::cleanUpWsa();
#endif
    SslSocket::cleanUpSsl();

    m_runnable = false;

    delete m_services;
}

int WebServer::exec()
{
    if(m_connections.empty())
        return -1;

    while(m_runnable)
    {
        this->eventHandler(m_epoll->epoll(m_interval));

        // Rempve timeout connections
        Socket socket = 0;
        while(m_timerManager.checkTop(socket))
            this->close(socket);
    }

    return 0;
}

void WebServer::listen(const std::string &hostName, const std::string &port,
                       bool sslEnable)
{
    if(!m_isLoaded)
    {
        ExceptionEvent event(ExceptionEvent::SocketLoadFailed);
        m_handler(&event);
        return;
    }

    if(sslEnable && !SslSocket::isSslAvailable())
    {
        ExceptionEvent event(ExceptionEvent::ListenFailed, "Listen "
            + hostName + ":" + port + " failed, SSL is not available.\n");
        m_handler(&event);
        return;
    }

    auto socket = std::make_shared<TcpSocket>();

    if(!socket->listen(hostName, port, sslEnable))
    {
        ExceptionEvent event(ExceptionEvent::ListenFailed, "Listen "
            + hostName + ":" + port + " failed, please rerun with an administrator.\n");
        m_handler(&event);
        return;
    }

    m_connections.insert(Connection(socket->descriptor(), socket));
    m_epoll->addConnection(socket->descriptor());
}

void WebServer::close(const Socket socket)
{
    m_epoll->removeConnection(socket);

    const auto it = m_connections.find(socket);

    if(it == m_connections.end())
        return;

    ConnectEvent event(it->second.get(), ConnectEvent::Close);
    m_handler(&event);

    it->second->timer()->deleteLater();
    m_connections.erase(it);
}

void WebServer::eventHandler(const EventList& list)
{
    const auto acceptConnection = [this](AbstractSocket * const connect) {
        if(!connect->isValid())
        {
            delete connect;
            return;
        }

        ConnectEvent event(connect, ConnectEvent::Accpet);
        m_handler(&event);

        connect->setTimer(m_timerManager.addTimer(connect->descriptor()));

        m_connections.insert(Connection(connect->descriptor(), connect));
        m_epoll->addConnection(connect->descriptor());
    };

    for(const auto &item : list)
    {
        if(item.events == 0)
            continue;

#ifdef _WIN32
        const Socket socket = item.fd;
#else
        const Socket socket = item.data.fd;
#endif

        const auto it = m_connections.find(socket);

        if(it == m_connections.end())
        {
            m_epoll->removeConnection(socket);
            continue;
        }

        if(item.events & ERROR_EVENT ||
            item.events & CLOSE_EVENT)
        {
            this->close(socket);
            continue;
        }

        if(it->second->isListening())
        {
            TcpSocket *listener = static_cast<TcpSocket *>(it->second.get());

            SocketInfo info;

            while(AbstractSocket::isValid(std::get<0>((info = listener->accept()))))
            {
                if(listener->sslEnable())
                    acceptConnection(new SslSocket(info));
                else
                    acceptConnection(new TcpSocket(info));
            }
        }
        else
        {
            std::string raw;

            it->second->read(raw);

            auto request = std::make_shared<HttpRequest>(raw);

            if(!request->isValid())
                continue;

            it->second->addTimes();

            auto temp = it->second;

            if(!request->isKeepAlive() || it->second->times() > m_maxTimes)
                this->close(socket);

            // TODO: ThreadPool
            std::thread(&WebServer::session, this, temp, request).detach();
            //m_pool.start(std::bind(&WebServer::session, this, temp, request));
        }
    }
}

void WebServer::session(std::shared_ptr<AbstractSocket> connect,
                        std::shared_ptr<HttpRequest> httpRequest)
{
    std::string response;
    std::shared_ptr<char[]> sendBuf(new char[SOCKET_BUF_SIZE]);

    auto httpResponse = std::make_shared<HttpResponse>();

    m_services->service(httpRequest.get(), httpResponse.get());

    connect->timer()->deleteLater();

    if(!httpRequest->isKeepAlive() || connect->times() > m_maxTimes)
        httpResponse->setRawHeader("Connection", "close");
    else    // Reset timer
        connect->setTimer(m_timerManager.addTimer(connect->descriptor()));

    httpResponse->toRawData(response);

    if(connect->write(response) <= 0)
        return;

    // Send file
    if(httpResponse->bodyType() == HttpResponse::File
        && httpRequest->method() == "GET")
    {
        std::ifstream out(httpResponse->filePath(), std::ios::binary);

        if(out)
        {
            std::string sendChunk;
            sendChunk.reserve(SOCKET_BUF_SIZE + 4);

            while(!out.eof())
            {
                out.read(sendBuf.get(), SOCKET_BUF_SIZE);

                Until::toHex(sendChunk, out.gcount());
                sendChunk.append("\r\n")
                    .append(sendBuf.get(), size_t(out.gcount()))
                    .append("\r\n");

                if(connect->write(sendChunk) <= 0)
                    return;

                sendChunk.clear();
            }
        }

        if(connect->write("0\r\n\r\n", 5) <= 0)     // End of chunk
            return;
    }
}
