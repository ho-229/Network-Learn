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
# define POLL(x, y, z) WSAPoll(x, y, z)
#else
# include <sys/select.h>
# include <poll.h>

# define POLL(x, y, z) poll(x, y, z)
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
        ExceptionEvent event(ExceptionEvent::ListenFailed, "Listen port "
            + m_port.first + " failed, please rerun with an administrator.\n");
        m_handler(&event);
        return -1;      // Start listen failed
    }

    // HTTPS listener
    if(m_sslEnable)
    {
        if(m_listenSockets.second->listen(m_port.second))
            FD_SET(m_listenSockets.second->descriptor(), &readSet);
        else
        {
            ExceptionEvent event(ExceptionEvent::ListenFailed, "Listen port "
                + m_port.second + " failed, please rerun with an administrator.\n");
            m_handler(&event);
            return -1;      // Start listen failed
        }
    }

    m_runnable = true;
    const auto maxfd = Until::max<TcpSocket>(m_listenSockets).descriptor() + 1;

    const auto acceptConnection = [this](const AcceptEvent::Protocol& protocol,
                                         AbstractSocket * const connect) {
        AcceptEvent event(protocol, connect->hostName(), connect->port());
        m_handler(&event);

        std::thread(&WebServer::session, this, connect).detach();
    };

    while(m_runnable)
    {
        readySet = readSet;
        if(select(int(maxfd), &readySet, nullptr, nullptr,
                   reinterpret_cast<timeval *>(&m_interval)) <= 0)
            continue;

        if(FD_ISSET(m_listenSockets.first->descriptor(), &readySet))    // HTTP
            acceptConnection(AcceptEvent::HTTP,
                             new TcpSocket(m_listenSockets.first->waitForAccept()));
        if(FD_ISSET(m_listenSockets.second->descriptor(), &readySet))   // HTTPS
            acceptConnection(AcceptEvent::HTTPS,
                             new SslSocket(m_listenSockets.second->waitForAccept()));
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
    std::shared_ptr<char[]> sendBuf(new char[SOCKET_BUF_SIZE]);

    for(int i = 0; i < 10; ++i)
    {
        connect->read(raw);

        if(raw.empty())
            break;

        auto httpRequest = std::make_shared<HttpRequest>(raw);
        auto httpResponse = std::make_shared<HttpResponse>();

        m_services->service(httpRequest.get(), httpResponse.get());
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

                connect->write("0\r\n\r\n", 5);
            }
        }

        pollfd pollFd = {connect->descriptor(), POLLIN, 0};
        if(POLL(&pollFd, 1, m_timeout) <= 0)
            break;
    }

    delete connect;
}
