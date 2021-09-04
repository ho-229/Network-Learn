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

void Epoll::addConnection(const Socket socket)
{
#ifdef _WIN32
    //m_addBuf.push_back(socket);
    m_events.push_back({socket, POLLIN, 0});
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
    epoll_ctl(m_epoll, EPOLL_CTL_DEL, socket, nullptr);
    --m_count;
#endif
}

const EventList Epoll::epoll(int interval)
{
#ifdef _WIN32
    // Remove invalid events
    if(!m_removeBuf.empty() && !m_events.empty())
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

    auto temp = m_events;

    if(temp.empty() ||
        WSAPoll(&temp[0], ULONG(temp.size()), interval) <= 0)
        return {};

    return temp;
#else   // Unix
    epoll_event events[MAX_EVENTS];

    int ret = -1;
    if(!m_count || (ret = epoll_wait(m_epoll, events, MAX_EVENTS, interval)) <= 0)
        return {};

    return {events, events + ret};
#endif
}
