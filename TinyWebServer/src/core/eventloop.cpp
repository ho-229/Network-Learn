/**
 * @author Ho 229
 * @date 2021/10/3
 */

#include "tcpsocket.h"
#include "sslsocket.h"
#include "eventloop.h"
#include "../abstract/abstractservices.h"

EventLoop::EventLoop(const volatile bool &runnable,
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

EventLoop::~EventLoop()
{
    while(auto socket = m_manager.takeFirst())
    {
        m_handler(new ConnectEvent(socket, ConnectEvent::Close));
        delete socket;
    }
}

void EventLoop::exec()
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

void EventLoop::processQueue()
{
    for(auto& socket : m_queue)
    {
        if(socket->isListener())
        {
            AbstractSocket *client = nullptr;

            while(socket->isValid())
            {
                const Socket descriptor = socket->accept();

                if(!AbstractSocket::isValid(descriptor))
                    break;

                client = socket->sslEnable() ?
                            static_cast<AbstractSocket *>(new SslSocket(descriptor)) :
                            static_cast<AbstractSocket *>(new TcpSocket(descriptor));

                if(!client->isValid())
                {
                    delete client;
                    continue;
                }

                client->setTimer(m_manager.start(m_timeout, client));
                m_epoll.insert(client);

                m_handler(new ConnectEvent(client, ConnectEvent::Accpet));
            }
        }
        else
        {
            if(m_services->process(socket))
                m_manager.restart(socket->timer());
            else    // Close
            {
                m_handler(new ConnectEvent(socket, ConnectEvent::Close));
                m_epoll.erase(socket);

                m_manager.destory(socket->timer());
                delete socket;
            }
        }
    }

    m_queue.resize(0);
}

void EventLoop::processErrorQueue()
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

            m_manager.destory(socket->timer());
            delete socket;
        }
    }

    m_errorQueue.resize(0);
}

void EventLoop::processTimeoutQueue()
{
    for(auto& socket : m_errorQueue)
    {
        m_epoll.erase(socket);
        m_handler(new ConnectEvent(socket, ConnectEvent::Close));

        delete socket;
    }

    m_errorQueue.resize(0);
}
