/**
 * @author Ho 229
 * @date 2021/10/3
 */

#include "tcpsocket.h"
#include "sslsocket.h"
#include "connectionpool.h"
#include "../abstract/abstractservices.h"

ConnectionPool::ConnectionPool(const std::atomic_bool &runnable,
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
    while(m_runnable)
    {
        m_epoll.epoll(m_queue, m_errorQueue);

        if(!m_queue.empty())
            this->processQueue();

        if(!m_errorQueue.empty())
            this->processErrorQueue();

        // Clean up timeout connections
        m_manager.checkout(m_errorQueue);

        if(!m_errorQueue.empty())
            this->processErrorQueue(false);
    }
}

void ConnectionPool::processQueue()
{
    for(auto& socket : m_queue)
    {
        if(socket->isListening())
        {
            while(true)
            {
                const Socket descriptor = static_cast<TcpSocket *>(socket)->accept();

                if(!AbstractSocket::isValid(descriptor))
                    break;

                AbstractSocket *newSocket = socket->sslEnable() ?
                                                static_cast<AbstractSocket *>(new SslSocket(descriptor)) :
                                                static_cast<AbstractSocket *>(new TcpSocket(descriptor));

                newSocket->setTimer(m_manager.addTimer(m_timeout, newSocket));
                m_epoll.insert(newSocket);

                ConnectEvent event(newSocket, ConnectEvent::Accpet);
                m_handler(&event);
            }
        }
        else
        {
            static_cast<decltype (m_manager)::TimerType *>(
                socket->timer())->deleteLater();

            if(m_services->process(socket))
                socket->setTimer(m_manager.addTimer(m_timeout, socket));   // Reset timer
            else    // Close
            {
                ConnectEvent event(socket, ConnectEvent::Close);
                m_handler(&event);

                m_epoll.erase(socket);
            }
        }
    }

    m_queue.clear();
}

void ConnectionPool::processErrorQueue(const bool deleteTimer)
{
    for(auto& socket : m_errorQueue)
    {
        ConnectEvent event(socket, ConnectEvent::Close);
        m_handler(&event);

        if(deleteTimer)
            static_cast<decltype (m_manager)::TimerType *>(
                socket->timer())->deleteLater();

        m_epoll.erase(socket);
    }

    m_errorQueue.clear();
}
