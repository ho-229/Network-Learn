/**
 * @author Ho 229
 * @date 2021/10/3
 */

#include "epoll.h"
#include "tcpsocket.h"
#include "sslsocket.h"
#include "connectionpool.h"
#include "../abstract/abstractservices.h"

ConnectionPool::ConnectionPool(const std::atomic_bool &runnable,
                               int timeout, int interval,
                               AbstractServices *const services,
                               const EventHandler &handler) :
    m_runnable(runnable),
    m_services(services),
    m_handler(handler),
    m_thread(std::bind(&ConnectionPool::exec, this, interval))
{
    m_connections.reserve(512);

    m_manager.setTimeout(timeout);
}

ConnectionPool::~ConnectionPool()
{

}

void ConnectionPool::addConnection(std::shared_ptr<AbstractSocket> socket)
{
    m_connections.insert(Connection(socket->descriptor(), socket));
    m_epoll.addConnection(socket->descriptor());
}

void ConnectionPool::exec(int interval)
{
    while(m_runnable)
    {
        this->eventsHandler(m_epoll.epoll(interval));

        // Clean up timeout connections
        Socket socket;
        while(m_manager.checkTop(socket))
        {
            const auto it = m_connections.find(socket);
            if(it != m_connections.end())
                this->release(it->second.get());
        }
    }
}

void ConnectionPool::eventsHandler(const EventList &events)
{
    for(const auto& item : events)
    {
        if(item.events == 0)
            continue;

#ifdef _WIN32
        const Socket fd = item.fd;
#else
        const Socket fd = item.data.fd;
#endif

        const auto it = m_connections.find(fd);

        if(it == m_connections.end())
            m_epoll.removeConnection(fd);
        else if(item.events & CLOSE_EVENT || item.events & ERROR_EVENT)
        {
            this->release(it->second.get());
            continue;
        }

        if(it->second->isListening())
        {
            while(true)
            {
                const SocketInfo info =
                    static_cast<TcpSocket *>(it->second.get())->accept();

                if(!AbstractSocket::isValid(info.descriptor))
                    break;

                AbstractSocket *socket = it->second->sslEnable() ?
                    static_cast<AbstractSocket *>(new SslSocket(info)) :
                    static_cast<AbstractSocket *>(new TcpSocket(info));

                socket->setTimer(m_manager.addTimer(socket->descriptor()));
                this->addConnection(std::shared_ptr<AbstractSocket>(socket));

                ConnectEvent event(socket, ConnectEvent::Accpet);
                m_handler(&event);
            }
        }
        else
        {
            if(m_services->service(it->second.get()))
            {
                it->second->timer()->deleteLater();
                it->second->setTimer(m_manager.addTimer(it->second->descriptor()));
            }
            else
                this->release(it->second.get());
        }
    }
}

void ConnectionPool::release(const AbstractSocket *socket)
{
    ConnectEvent event(socket, ConnectEvent::Close);
    m_handler(&event);

    socket->timer()->deleteLater();
    m_epoll.removeConnection(socket->descriptor());
    m_connections.erase(socket->descriptor());
}
