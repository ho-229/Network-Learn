/**
 * @author Ho 229
 * @date 2021/8/20
 */

#include "epoll.h"

#include <iostream>

Epoll::Epoll()
{
#ifndef _WIN32
    m_epoll = epoll_create1(0);
#endif
}

Epoll::~Epoll()
{
#ifndef _WIN32
    close(m_epoll);
#endif
}

void Epoll::addConnection(const Socket socket)
{
#ifdef _WIN32
    m_addBuf.push_back(socket);
#else   // Unix
    epoll_event newEvent{};
    newEvent.events = EPOLLIN | EPOLLET;
    newEvent.data.fd = socket;

    epoll_ctl(m_epoll, EPOLL_CTL_ADD, socket, &newEvent);

    ++m_count;
#endif
}

void Epoll::removeConnection(const Socket socket)
{
#ifdef _WIN32
    m_removeBuf.push_back(socket);
#else
    //epoll_ctl(m_epoll, EPOLL_CTL_DEL, socket, nullptr);
    --m_count;
#endif
}

/*void Epoll::exec(int interval, const SessionHandler &handler)
{
#ifdef _WIN32
    std::vector<Socket> invalidEvents;
#else
    std::shared_ptr<epoll_event[]> events(new epoll_event[MAX_EVENTS]());
#endif
    while(m_runnable)
    {
        if(m_connections.empty())
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condition.wait(lock);
        }

        // Check timeout
        m_mutex.lock();
        Socket timeoutSocket = 0;
        while(m_timerManager.checkTop(timeoutSocket))
        {
            this->removeConnection(timeoutSocket);
#ifdef _WIN32
            invalidEvents.push_back(timeoutSocket);
#endif
        }
        m_mutex.unlock();

#ifdef _WIN32
        // Remove invalid events
        if(!invalidEvents.empty())
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            for(auto it = m_events.begin(); it < m_events.end();)
            {
                auto invIt = std::find(invalidEvents.begin(),
                                       invalidEvents.end(), it->fd);

                if(invIt != invalidEvents.end())
                {
                    m_events.erase(it);
                    invalidEvents.erase(invIt);
                }
                else
                    ++it;
            }
        }

        m_mutex.lock();
        auto temp = m_events;
        m_mutex.unlock();

        if(m_events.empty()
            || WSAPoll(&temp[0], ULONG(temp.size()), interval) <= 0)
            continue;

        for(auto it = temp.begin(); it != temp.end(); ++it)
        {
            if (it->revents & POLLHUP || it->revents & POLLNVAL ||
                it->revents & POLLERR)
            {
                invalidEvents.push_back(it->fd);
                continue;
            }
            else if(it->revents & POLLIN)
            {
                auto connIt = m_connections.find(it->fd);
                if(connIt == m_connections.end())
                {
                    invalidEvents.push_back(it->fd);
                    continue;
                }

                AbstractSocket * const socket = connIt->second.get();
                socket->addTimes();

                if(!handler(socket) || socket->times() == m_maxTimes)
                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    this->removeConnection(it->fd);
                    invalidEvents.push_back(it->fd);
                    continue;
                }
                else
                {
                    // Reset timer
                    std::unique_lock<std::mutex> lock(m_mutex);
                    socket->timer()->deleteLater();
                    socket->setTimer(m_timerManager.addTimer(it->fd));
                }
            }
        }
#else   // Unix
        // Wait for network events
        int ret = -1;
        if(m_connections.empty() ||
                (ret = epoll_wait(m_epoll, events.get(), MAX_EVENTS, interval)) <= 0)
            continue;

        for(int i = 0; i < ret; ++i)
        {
            auto it = m_connections.find(events[i].data.fd);
            if(it == m_connections.end())
            {
                epoll_ctl(m_epoll, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
                continue;
            }

            AbstractSocket * const socket = it->second.get();
            socket->addTimes();

            if(!handler(socket) || socket->times() == m_maxTimes)   // Process
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                this->removeConnection(socket->descriptor());
            }
            else
            {
                // Reset timer
                std::unique_lock<std::mutex> lock(m_mutex);
                socket->timer()->deleteLater();
                socket->setTimer(m_timerManager.addTimer(socket->descriptor()));
            }
        }
#endif
    }
}*/

void Epoll::process(int interval, const SessionHandler &handler)
{
#ifdef _WIN32
    // Remove invalid events
    if(!m_removeBuf.empty())
    {
        for(auto it = m_events.begin(); it != m_events.end();)
        {
            auto invIt = std::find(m_removeBuf.begin(),
                                   m_removeBuf.end(), it->fd);

            if(invIt != m_removeBuf.end())
            {
                m_events.erase(it);
                m_removeBuf.erase(invIt);
            }
            else
                ++it;
        }
        m_removeBuf.clear();
    }

    // Append new events
    if(!m_addBuf.empty())
    {
        for(const auto& sock : m_addBuf)
            m_events.push_back({sock, POLLIN, 0});
        m_addBuf.clear();
    }

    // Wait for events
    if(m_events.empty() ||
        WSAPoll(&m_events[0], ULONG(m_events.size()), interval) <= 0)
        return;

    for(auto it = m_events.cbegin(); it != m_events.cend();)
    {
        if (it->revents & POLLHUP ||
            it->revents & POLLNVAL ||
            it->revents & POLLERR)
        {   // Invalid event
            handler(it->fd, false);
            m_events.erase(it);
            continue;
        }
        else if(it->revents & POLLIN)
            handler(it->fd, true);

        ++it;
    }
#else
    std::shared_ptr<epoll_event[]> events(new epoll_event[MAX_EVENTS]());

    int ret = -1;
    if(!m_count || (ret = epoll_wait(m_epoll, events.get(), MAX_EVENTS, interval)) <= 0)
        return;

    for(int i = 0; i < ret; ++i)
    {
        const auto& event = events[i];
        if(event.events & EPOLLERR)
        {
            std::cout << "ERR: "<<event.data.fd<<"\n";
            handler(event.data.fd, false);
            this->removeConnection(event.data.fd);
        }
        else if(event.events & EPOLLIN)
            handler(event.data.fd, true);
    }
#endif
}
