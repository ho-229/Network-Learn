/**
 * @author Ho 229
 * @date 2021/7/26
 */

#include "webserver.h"

#define SOCKET_BUF_SIZE 4096

#include "until.h"
#include "event.h"
#include "tcpsocket.h"
#include "sslsocket.h"
#include "httpservices.h"

#include <thread>
#include <future>
#include <fstream>

#include <signal.h>

#ifdef _WIN32
# include <WinSock2.h>
#else
# include <sys/select.h>
#endif

WebServer::WebServer() :
    m_listenSockets({new TcpSocket(), new TcpSocket()}),
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

    delete m_listenSockets.first;
    delete m_listenSockets.second;

    delete m_services;
}

int WebServer::exec()
{
    if(!m_isLoaded)
    {
        ExceptionEvent event(ExceptionEvent::SocketLoadFailed);
        m_handler(&event);
        return -1;
    }

    fd_set readSet, readySet;
    FD_ZERO(&readSet);

    // HTTP listener
    if(m_listenSockets.first->listen(m_port.first))
        FD_SET(m_listenSockets.first->descriptor(), &readSet);
    else
    {
        ExceptionEvent event(ExceptionEvent::ListenFailed);
        m_handler(&event);
        return -1;      // Start listen failed
    }

    // HTTPS listener
    if(m_sslEnable && m_listenSockets.second->listen(m_port.second))
        FD_SET(m_listenSockets.second->descriptor(), &readSet);
    else
    {
        ExceptionEvent event(ExceptionEvent::ListenFailed);
        m_handler(&event);
        return -1;      // Start listen failed
    }

    m_runnable = true;
    const auto maxfd = Until::max<TcpSocket>(m_listenSockets).descriptor() + 1;

    const auto acceptConnection = [this](AbstractSocket * const connect) {
        AcceptEvent event(connect->hostName(), connect->port());
        m_handler(&event);

        auto future = std::async(&WebServer::session, this, connect);
    };

    while(m_runnable)
    {
        readySet = readSet;
        if(select(int(maxfd), &readySet, nullptr, nullptr,
                   reinterpret_cast<timeval *>(&m_timeout)) <= 0)
            continue;

        if(FD_ISSET(m_listenSockets.first->descriptor(), &readySet))
            acceptConnection(new TcpSocket(m_listenSockets.first->waitForAccept()));
        if(FD_ISSET(m_listenSockets.second->descriptor(), &readySet))
            acceptConnection(new SslSocket(m_listenSockets.second->waitForAccept()));
    }

    return 0;
}

void WebServer::setSslEnable(bool enable)
{
    m_sslEnable = enable && SslSocket::isSslAvailable();
}

void WebServer::session(AbstractSocket * const connect)
{
    std::string raw, response;

    connect->read(raw);

    if(raw.empty())
    {
        delete connect;
        return;
    }

    auto httpRequest = std::make_shared<HttpRequest>(raw);
    auto httpResponse = std::make_shared<HttpResponse>();

    m_services->service(httpRequest.get(), httpResponse.get());

    httpResponse->toRawData(response);
    connect->AbstractSocket::write(response);

    if(httpResponse->bodyType() == HttpResponse::File
        && httpRequest->method() == "GET")
    {
        std::shared_ptr<char[]> sendBuf(new char[SOCKET_BUF_SIZE]());
        std::ifstream out(httpResponse->filePath(), std::ios::binary);

        if(out)
        {
            const auto range = httpRequest->range();

            if(range.second > 0 && range.first != range.second)
            {
                out.seekg(range.first);
                while(out.tellg() < range.second)
                {
                    out.read(sendBuf.get(),
                             out.tellg() + int64_t(SOCKET_BUF_SIZE) > range.second
                                 ? range.second - out.tellg() : SOCKET_BUF_SIZE);
                    if(connect->write(sendBuf.get(), size_t(out.gcount())) <= 0)
                        break;
                }
            }
            else
            {
                while(!out.eof())
                {
                    out.read(sendBuf.get(), SOCKET_BUF_SIZE);
                    if(connect->write(sendBuf.get(), size_t(out.gcount())) <= 0)
                        break;
                }
            }
        }
    }

    delete connect;
}
