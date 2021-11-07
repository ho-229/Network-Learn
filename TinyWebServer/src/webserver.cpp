/**
 * @author Ho 229
 * @date 2021/7/26
 */

#include "webserver.h"

#include "util/util.h"
#include "core/tcpsocket.h"
#include "core/sslsocket.h"
#include "core/connectionpool.h"

#include <signal.h>

WebServer::WebServer()
{
#ifdef _WIN32
    m_isLoaded = TcpSocket::initializatWsa();
#else   // Unix
    // Ignore SIGPIPE
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, 0);
#endif
}

WebServer::~WebServer()
{
#ifdef _WIN32
    TcpSocket::cleanUpWsa();
#endif
    SslSocket::cleanUpSsl();

    m_runnable = false;
}

int WebServer::start()
{
    if(m_listeners.empty() || !m_services || !m_pools.empty())
        return -1;

    ConnectionPool *pool = nullptr;
    for(size_t i = 0; i < m_loopCount; ++i)
    {
        m_pools.emplace_back(pool = new ConnectionPool(
            m_runnable, m_timeout, m_services.get(), m_handler));

        for(const auto& connect : m_listeners)
            pool->registerListener(connect.get());

        pool->start();
    }

    return 0;
}

void WebServer::waitForFinished()
{   
    for(auto &pool : m_pools)
        pool->waitForFinished();

    m_pools.clear();
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
