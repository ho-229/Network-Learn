﻿/**
 * @author Ho 229
 * @date 2021/7/26
 */

#include "webserver.h"

#include "until/until.h"
#include "core/tcpsocket.h"
#include "core/sslsocket.h"
#include "core/connectionpool.h"

#include <signal.h>

WebServer::WebServer()
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

int WebServer::start()
{
    if(m_listeners.empty() || !m_services || !m_pools.empty())
        return -1;

    for(size_t i = 0; i < m_loopCount; ++i)
    {
        m_pools.emplace_back(new ConnectionPool(
            m_runnable, m_timeout, m_interval, m_services.get(), m_handler));

        for(const auto& connect : m_listeners)
            m_pools.back()->registerListener(connect.get());
    }

    return 0;
}

void WebServer::waitForFinished()
{   
    for(auto &pool : m_pools)
    {
        if(pool->thread().joinable())
            pool->thread().join();
    }

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
