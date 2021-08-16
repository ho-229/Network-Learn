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
        FD_SET(item.first->descriptor(), &readSet);

    m_runnable = true;
    const auto maxfd = m_listeners.back().first->descriptor() + 1;

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

        for(auto& item : m_listeners)
        {
            if(FD_ISSET(item.first->descriptor(), &readySet))
            {
                if(item.second)
                    acceptConnection(AcceptEvent::HTTPS,
                                     new SslSocket(item.first->waitForAccept()));
                else
                    acceptConnection(AcceptEvent::HTTP,
                                     new TcpSocket(item.first->waitForAccept()));
            }
        }
    }

    return 0;
}

void WebServer::listen(const std::string &hostName, const std::string &port, bool sslEnable)
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

    if(!socket->listen(hostName, port))
    {
        ExceptionEvent event(ExceptionEvent::ListenFailed, "Listen "
            + hostName + ":" + port + " failed, please rerun with an administrator.\n");
        m_handler(&event);
        return;
    }

    m_listeners.push_back({socket, sslEnable});
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
