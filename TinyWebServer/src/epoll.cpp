/**
 * @author Ho 229
 * @date 2021/8/20
 */

#include "epoll.h"

#include <iostream>

Epoll::Epoll()
{

}

Epoll::~Epoll()
{

}

void Epoll::addConnection(AbstractSocket* socket)
{
    std::unique_lock<std::mutex> lock(m_mutex);
#ifdef _WIN32
    m_events.push_back({socket->descriptor(), POLLIN, 0});
#else   // Unix
    return;     // TODO
#endif
    m_connections.insert(Connection(socket->descriptor(), socket));

    socket->setTimer(m_timerManager.addTimer(socket->descriptor()));

    m_condition.notify_one();
}

void Epoll::exec(int interval, const SessionHandler &handler)
{
    while(m_runnable)
    {
        if(m_connections.empty())
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condition.wait(lock);
        }

#ifdef _WIN32
        auto temp = m_events;

        if(WSAPoll(&temp[0], ULONG(temp.size()), interval) <= 0)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            for(auto it = m_events.begin(); it != m_events.end();)
            {
                if(m_timerManager.checkTop(it->fd))
                {
                    this->removeConnection(it->fd);
                    it = m_events.erase(it);
                }
                else
                    ++it;
            }

            continue;
        }

        size_t i = 0;

        const auto removeEvent = [&] {
            if(i < m_events.size())
                m_events.erase(m_events.begin() + int(i));
        };

        for(auto it = temp.begin(); it != temp.end(); ++it)
        {
            if (it->revents & POLLHUP || it->revents & POLLNVAL ||
                it->revents & POLLERR)
			{
				removeEvent();
                continue;
			}
            else if(it->revents & POLLIN)
            {
                auto connIt = m_connections.find(it->fd);
                if(connIt == m_connections.end())
                {
                    removeEvent();
                    continue;
                }

                AbstractSocket * const socket = connIt->second.get();
                socket->addTimes();

                if(!handler(socket) || socket->times() == m_maxTimes)
                {
                    this->removeConnection(it->fd);
                    removeEvent();
                    continue;
                }
                else    // Reset timer
                {
                    socket->timer()->isDisable = true;
                    socket->setTimer(m_timerManager.addTimer(it->fd));
                }
            }

            if(m_timerManager.checkTop(it->fd))
            {
                this->removeConnection(it->fd);
                removeEvent();
                continue;
            }

            ++i;
        }
#else   // Unix
        return;     // TODO
#endif
    }
}

void Epoll::removeConnection(const Socket &socket)
{
    m_connections[socket]->timer()->isDisable = true;
    m_connections.erase(socket);

    std::cout << m_connections.size() << "\n";
#ifdef _WIN32

#endif
}
