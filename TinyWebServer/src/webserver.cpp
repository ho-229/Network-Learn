/**
 * @author Ho 229
 * @date 2021/7/26
 */

#include "webserver.h"

#include "util/util.h"
#include "core/tcpsocket.h"
#include "core/sslsocket.h"
#include "core/eventloop.h"

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
    if(m_listeners.empty() || !m_services || !m_loops.empty())
        return -1;

    m_runnable = true;

    EventLoop *pool = nullptr;
    for(size_t i = 0; i < m_loopCount; ++i)
    {
        m_loops.emplace_back(pool = new EventLoop(
            m_runnable, m_timeout, m_services.get(), m_handler));

        pool->registerListeners(m_listeners.begin(), m_listeners.end());
        pool->start();
    }

    return 0;
}

void WebServer::requestQuit()
{
    for(auto &loop : m_loops)
        loop->unregisterListeners(m_listeners.begin(), m_listeners.end());

    for(auto &listener : m_listeners)
        listener->close();
}

void WebServer::waitForFinished()
{
    for(auto &loop : m_loops)
        loop->waitForFinished();

    m_listeners.clear();
    m_loops.clear();
}

bool WebServer::listen(const std::string &hostName, const std::string &port,
                       bool sslEnable, std::pair<int, int> linger)
{
    if(sslEnable && !SslSocket::isSslAvailable())
        return false;

    std::unique_ptr<AbstractSocket> socket(sslEnable ?
            static_cast<AbstractSocket *>(new SslSocket()) :
            static_cast<AbstractSocket *>(new TcpSocket()));

    if(!socket->listen(hostName, port))
        return false;

    socket->setOption(SOL_SOCKET, SO_LINGER, linger);

    m_listeners.emplace_back(std::move(socket));

    return true;
}
