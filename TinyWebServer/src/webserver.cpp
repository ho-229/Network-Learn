/**
 * @author Ho 229
 * @date 2021/7/26
 */

#include "webserver.h"

#include "until.h"
#include "tcpsocket.h"
#include "sslsocket.h"

#include <signal.h>

#ifdef _WIN32
# include <WinSock2.h>
#else
# include <sys/select.h>
#endif

WebServer::WebServer()
    //m_pool(std::thread::hardware_concurrency())
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
}

int WebServer::exec()
{
    if(m_listeners.empty() || !m_services)
        return -1;

    for(size_t i = 0; i < m_loopCount; ++i)
    {
        m_pools.emplace_back(std::make_shared<ConnectionPool>());

        for(const auto& connect : m_listeners)
            m_pools.back()->addConnection(connect);

        m_pools.back()->setServices(m_services.get());
        m_pools.back()->setTimeout(m_timeout);
        m_pools.back()->installEventHandler(m_handler);
    }

    std::vector<std::thread> workers;

    for(size_t i = 0; i < m_loopCount; ++i)
    {
        workers.push_back(std::thread(
            std::bind(&ConnectionPool::exec, m_pools[i].get(), m_interval)));
    }

    // Wait for finished
    for(auto& worker : workers)
    {
        if(worker.joinable())
            worker.join();
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


