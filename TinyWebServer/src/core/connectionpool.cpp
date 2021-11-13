/**
 * @author Ho 229
 * @date 2021/10/3
 */

#include "tcpsocket.h"
#include "sslsocket.h"
#include "connectionpool.h"
#include "../abstract/abstractservices.h"

ConnectionPool::ConnectionPool(const std::atomic_bool &runnable, int timeout,
                               AbstractServices *const services,
                               const EventHandler &handler) :
    m_runnable(runnable),
    m_services(services),
    m_handler(handler)
{
    m_manager.setTimeout(timeout);
    m_queue.reserve(512);
}

ConnectionPool::~ConnectionPool()
{

}

void ConnectionPool::exec()
{
    while(m_runnable)
    {
        m_epoll.epoll(m_queue, [this](AbstractSocket *const socket) {
            ConnectEvent event(socket, ConnectEvent::Close);
            m_handler(&event);

            socket->timer()->deleteLater();
            m_epoll.erase(socket);
        });

        if(!m_queue.empty())
            this->eventsHandler();

        // Clean up timeout connections
        AbstractSocket *socket = nullptr;
        while(m_manager.checkTop(socket))
        {
            ConnectEvent event(socket, ConnectEvent::Close);
            m_handler(&event);

            m_epoll.erase(socket);
        }
    }
}

void ConnectionPool::eventsHandler()
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

                newSocket->setTimer(m_manager.addTimer(newSocket));
                m_epoll.insert(newSocket);

                ConnectEvent event(newSocket, ConnectEvent::Accpet);
                m_handler(&event);
            }
        }
        else
        {
            socket->timer()->deleteLater();
            if(m_services->process(socket))
                socket->setTimer(m_manager.addTimer(socket));   // Reset timer
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
