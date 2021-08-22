/**
 * @author Ho 229
 * @date 2021/8/20
 */

#include "epoll.h"

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

void Epoll::addConnection(AbstractSocket* socket)
{
    std::unique_lock<std::mutex> lock(m_mutex);
#ifdef _WIN32
    m_events.push_back({socket->descriptor(), POLLIN, 0});
#else   // Unix
    epoll_event newEvent{};
    newEvent.events = EPOLLIN | EPOLLET;
    newEvent.data.fd = socket->descriptor();

    epoll_ctl(m_epoll, EPOLL_CTL_ADD, socket->descriptor(), &newEvent);
#endif
    m_connections.insert(Connection(socket->descriptor(), socket));

    socket->setTimer(m_timerManager.addTimer(socket->descriptor()));

    m_condition.notify_one();
}

void Epoll::exec(int interval, const SessionHandler &handler)
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

#ifdef _WIN32
        Socket timeoutSocket = 0;

        while(m_timerManager.checkTop(timeoutSocket))
        {
            invalidEvents.push_back(timeoutSocket);
            this->removeConnection(timeoutSocket);
        }

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
            continue;
        }

        m_mutex.lock();
        auto temp = m_events;
        m_mutex.unlock();

        if(WSAPoll(&temp[0], ULONG(temp.size()), interval) <= 0)
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
                    this->removeConnection(it->fd);
                    invalidEvents.push_back(it->fd);
                    continue;
                }
                else    // Reset timer
                {
                    socket->timer()->isDisable = true;
                    socket->setTimer(m_timerManager.addTimer(it->fd));
                }
            }
        }
#else   // Unix

        // Check timeout
        bool ok = false;
        while(true)
        {
            const Socket fd = m_timerManager.checkTop(ok);
            if(ok)
                this->removeConnection(fd);
            else
                break;
        }

        if(m_connections.empty())
            continue;

        // Wait for network events
        int ret = -1;
        if((ret = epoll_wait(m_epoll, events.get(), MAX_EVENTS, interval)) <= 0)
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
                this->removeConnection(socket->descriptor());
            else
            {
                // Reset timer
                socket->timer()->isDisable = true;
                socket->setTimer(m_timerManager.addTimer(socket->descriptor()));
            }
        }
#endif
    }
}

void Epoll::removeConnection(const Socket &socket)
{
    auto it = m_connections.find(socket);

    if(it == m_connections.end())
        return;

    it->second->timer()->isDisable = true;

    std::unique_lock<std::mutex> lock(m_mutex);
    m_connections.erase(it);
}
