/**
 * @author Ho 229
 * @date 2021/10/3
 */

#include "tcpsocket.h"
#include "sslsocket.h"
#include "connectionpool.h"
#include "../abstract/abstractservices.h"

ConnectionPool::ConnectionPool(const volatile bool &runnable,
                               const std::chrono::milliseconds &timeout,
                               AbstractServices *const services,
                               const EventHandler &handler) :
    m_runnable(runnable),
    m_timeout(timeout),
    m_services(services),
    m_handler(handler)
{
    m_queue.reserve(512);
    m_errorQueue.reserve(512);
}

ConnectionPool::~ConnectionPool()
{

}

void ConnectionPool::exec()
{
    while(m_runnable && m_epoll.count())
    {
        m_epoll.epoll(m_queue, m_errorQueue);

        if(!m_queue.empty())
            this->processQueue();

        if(!m_errorQueue.empty())
            this->processErrorQueue();

        // Clean up timeout connections
        m_manager.checkout(m_errorQueue);

        if(!m_errorQueue.empty())
            this->processTimeoutQueue();
    }
}

void ConnectionPool::processQueue()
{
    for(auto& socket : m_queue)
    {
        if(socket->isListener())
        {
            while(socket->isValid())
            {
                const Socket descriptor = socket->accept();

                if(!AbstractSocket::isValid(descriptor))
                    break;

                AbstractSocket *newSocket = socket->sslEnable() ?
                                                static_cast<AbstractSocket *>(new SslSocket(descriptor)) :
                                                static_cast<AbstractSocket *>(new TcpSocket(descriptor));

                newSocket->setTimer(m_manager.addTimer(m_timeout, newSocket));
                m_epoll.insert(newSocket);

                m_handler(new ConnectEvent(newSocket, ConnectEvent::Accpet));
            }
        }
        else
        {
            if(auto timer = static_cast<decltype(m_manager)::TimerType *>(socket->timer()))
                timer->deleteLater();

            if(m_services->process(socket))
                socket->setTimer(m_manager.addTimer(m_timeout, socket));   // Reset timer
            else    // Close
            {
                m_handler(new ConnectEvent(socket, ConnectEvent::Close));
                m_epoll.erase(socket);

                delete socket;
            }
        }
    }

    m_queue.resize(0);
}

void ConnectionPool::processErrorQueue()
{
    for(auto& socket : m_errorQueue)
    {
        m_epoll.erase(socket);

        if(socket->isListener())
        {
            m_handler(new ExceptionEvent(ExceptionEvent::ListenerError,
                                         "An error occurred in the listener"));
            continue;
        }
        else
        {
            m_handler(new ConnectEvent(socket, ConnectEvent::Close));

            if(auto timer = static_cast<decltype(m_manager)::TimerType *>(socket->timer()))
                timer->deleteLater();

            delete socket;
        }
    }

    m_errorQueue.resize(0);
}

void ConnectionPool::processTimeoutQueue()
{
    for(auto& socket : m_errorQueue)
    {
        m_epoll.erase(socket);
        m_handler(new ConnectEvent(socket, ConnectEvent::Close));

        delete socket;
    }

    m_errorQueue.resize(0);
}
