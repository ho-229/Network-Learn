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
    if(!AbstractSocket::initializatWsa())
        throw std::runtime_error("WebServer: initializat WinSock2 failed.");
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

    m_runnable = true;

    ConnectionPool *pool = nullptr;
    for(size_t i = 0; i < m_loopCount; ++i)
    {
        m_pools.emplace_back(pool = new ConnectionPool(
            m_runnable, m_timeout, m_services.get(), m_handler));

        pool->registerListeners(m_listeners.begin(), m_listeners.end());
        pool->start();
    }

    return 0;
}

void WebServer::requestQuit()
{
    for(auto &pool : m_pools)
        pool->unregisterListeners(m_listeners.begin(), m_listeners.end());
}

void WebServer::waitForFinished()
{
    for(auto &pool : m_pools)
        pool->waitForFinished();

    m_listeners.clear();
    m_pools.clear();
}

void WebServer::listen(const std::string &hostName, const std::string &port,
                       bool sslEnable, std::pair<int, int> linger)
{
    if(sslEnable && !SslSocket::isSslAvailable())
    {
        m_handler(new ExceptionEvent(
            ExceptionEvent::ListenerError,
            "Listen " + hostName + ":" + port + " failed, SSL is not available.\n"));
        return;
    }

    std::unique_ptr<AbstractSocket> socket(sslEnable ?
            static_cast<AbstractSocket *>(new SslSocket()) :
            static_cast<AbstractSocket *>(new TcpSocket()));

    if(!socket->listen(hostName, port))
    {
        m_handler(new ExceptionEvent(
            ExceptionEvent::ListenerError,
            "Listen " + hostName + ":" + port + " failed, please rerun with an administrator.\n"));
        return;
    }

    socket->setOption(SOL_SOCKET, SO_LINGER, linger);

    m_listeners.emplace_back(std::move(socket));
}
