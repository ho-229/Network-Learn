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

#include <signal.h>

#ifdef _WIN32
# include <WinSock2.h>
#else
# include <sys/select.h>
#endif

WebServer::WebServer() :
    m_threadCount(std::thread::hardware_concurrency()),
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
    if(m_listeners.empty())
        return -1;

    fd_set readSet, readySet;
    FD_ZERO(&readSet);

    for(auto& item : m_listeners)
        FD_SET(item->descriptor(), &readSet);

    m_runnable = true;
    const auto maxfd = m_listeners.back()->descriptor() + 1;

    for(size_t i = 0; i < m_threadCount; ++i)
    {
        auto epoll = std::make_shared<Epoll>();

        epoll->setMaxTimes(m_maxTimes);
        epoll->setTimeout(m_timeout);
        epoll->installEventHandler(m_handler);

        m_epolls.push_back(
            {std::thread(&Epoll::exec, epoll.get(), 500,
                         std::bind(&WebServer::session, this, std::placeholders::_1)
                         ),
             epoll});
    }

    size_t index = 0;
    const auto acceptConnection = [this, &index](AbstractSocket * const connect) {
        if(!connect->isValid())
        {
            delete connect;
            return;
        }

        ConnectEvent event(connect, ConnectEvent::Accpet);
        m_handler(&event);

        m_epolls[index].second->addConnection(connect);

        if(index == m_threadCount - 1)
            index = 0;
        else
            ++index;
    };

    while(m_runnable)
    {
        readySet = readSet;
        if(select(int(maxfd), &readySet, nullptr, nullptr,
                   reinterpret_cast<timeval *>(&m_interval)) <= 0)
            continue;

        for(const auto &item : m_listeners)
        {
            if(FD_ISSET(item->descriptor(), &readySet))
            {
                if(item->sslEnable())
                    acceptConnection(new SslSocket(item->waitForAccept()));
                else
                    acceptConnection(new TcpSocket(item->waitForAccept()));
            }
        }
    }

    for(auto &item : m_epolls)
    {
        item.second->quit();
        item.first.join();
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

    m_listeners.push_back(socket);
}

bool WebServer::session(AbstractSocket * const connect) const
{
    std::string raw, response;
    std::shared_ptr<char[]> sendBuf(new char[SOCKET_BUF_SIZE]);

    connect->read(raw);

    if(raw.empty())
        return false;

    auto httpRequest = std::make_shared<HttpRequest>(raw);

    if(!httpRequest->isValid())
        return false;

    auto httpResponse = std::make_shared<HttpResponse>();

    m_services->service(httpRequest.get(), httpResponse.get());

    if(!httpRequest->isKeepAlive() || connect->times() == m_maxTimes)
        httpResponse->setRawHeader("Connection", "close");

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

    return httpRequest->isKeepAlive();
}
