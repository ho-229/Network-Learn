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
    m_listeners.clear();
}

void WebServer::listen(const std::string &hostName, const std::string &port,
                       bool sslEnable, std::pair<int, int> linger)
{
    if(!m_isLoaded)
    {
        m_handler(new ExceptionEvent(ExceptionEvent::SocketLoadError));
        return;
    }

    if(sslEnable && !SslSocket::isSslAvailable())
    {
        m_handler(new ExceptionEvent(
            ExceptionEvent::ListenerError,
            "Listen " + hostName + ":" + port + " failed, SSL is not available.\n"));
        return;
    }

    auto socket = new TcpSocket();

    if(!socket->listen(hostName, port, sslEnable))
    {
        m_handler(new ExceptionEvent(
            ExceptionEvent::ListenerError,
            "Listen " + hostName + ":" + port + " failed, please rerun with an administrator.\n"));
        return;
    }

    socket->setOption(SOL_SOCKET, SO_LINGER, linger);

    m_listeners.emplace_back(static_cast<AbstractSocket *>(socket));
}
