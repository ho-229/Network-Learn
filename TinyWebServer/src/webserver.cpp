/**
 * @author Ho 229
 * @date 2021/7/26
 */

#include "webserver.h"

#include "epoll.h"
#include "until.h"
#include "event.h"
#include "tcpsocket.h"
#include "sslsocket.h"
#include "httpservices.h"

#include <fstream>
#include <iostream>

#include <signal.h>

#ifdef _WIN32
# include <WinSock2.h>
#else
# include <sys/select.h>
#endif

WebServer::WebServer() :
    m_threadCount(std::thread::hardware_concurrency()),
    m_epoll(new Epoll),
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
        m_epoll->process(m_interval,
                         std::bind(&WebServer::readableHandler, this,
                                   std::placeholders::_1,
                                   std::placeholders::_2));

        // Rempve timeout connections
        Socket socket = 0;
        while(m_timerManager.checkTop(socket))
        {
            std::cout << "timeout\n";
            this->popConnection(socket);
        }
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

void WebServer::popConnection(const Socket socket)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    const auto it = m_connections.find(socket);

    if(it == m_connections.end())
        return;

    ConnectEvent event(it->second.get(), ConnectEvent::Close);
    m_handler(&event);

    it->second->timer()->deleteLater();
    m_epoll->removeConnection(it->first);
    m_connections.erase(it);
}

void WebServer::readableHandler(const Socket socket, bool isAvailable)
{
    const auto it = m_connections.find(socket);
    if(it == m_connections.end())
    {
        m_epoll->removeConnection(socket);
        return;
    }
    else if(!isAvailable)
    {
        m_connections.erase(it);
        return;
    }

    if(it->second->isListening())
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

            //m_mutex.lock();
            m_connections.insert(Connection(connect->descriptor(), connect));
            m_epoll->addConnection(connect->descriptor());
            //m_mutex.unlock();
        };

        TcpSocket *listener = static_cast<TcpSocket *>(it->second.get());

        if(listener->sslEnable())
            acceptConnection(new SslSocket(listener->waitForAccept()));
        else
            acceptConnection(new TcpSocket(listener->waitForAccept()));
    }
    else
    {
        this->session(it->second);
        //std::thread(&WebServer::session, this, it->second).detach();
    }
}

bool WebServer::session(std::shared_ptr<AbstractSocket> connect)
{
    std::string raw, response;
    std::shared_ptr<char[]> sendBuf(new char[SOCKET_BUF_SIZE]);

    connect->read(raw);

    if(raw.empty())
    {
        std::cout << "empty\n";
        this->popConnection(connect->descriptor());
        return false;
    }

    auto httpRequest = std::make_shared<HttpRequest>(raw);

    if(!httpRequest->isValid())
    {
        std::cout << "invalid\n";
        this->popConnection(connect->descriptor());
        return false;
    }

    // Reset timer
    connect->timer()->deleteLater();
    connect->setTimer(m_timerManager.addTimer(connect->descriptor()));

    auto httpResponse = std::make_shared<HttpResponse>();

    m_services->service(httpRequest.get(), httpResponse.get());

    if(!httpRequest->isKeepAlive() || connect->times() == m_maxTimes)
    {
        std::cout << "start pop connection\n";
        httpResponse->setRawHeader("Connection", "close");
        this->popConnection(connect->descriptor());
        std::cout << "end pop connection\n";
    }

    httpResponse->toRawData(response);
    connect->write(response);

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

                connect->write(sendChunk);

                sendChunk.clear();
            }
        }

        connect->write("0\r\n\r\n", 5);     // End of chunk
    }

    return true;
}
