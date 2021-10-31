/**
 * @author Ho 229
 * @date 2021/8/20
 */

#include "epoll.h"
#include "../until/timermanager.h"

Epoll::Epoll()
{
#ifdef _WIN32
    m_removeBuf.reserve(128);
#else
    m_epoll = epoll_create1(0);
#endif
}

Epoll::~Epoll()
{
#ifndef _WIN32
    close(m_epoll);
#endif
}

void Epoll::insert(AbstractSocket * const socket, bool once)
{
#ifdef _WIN32
    // Win32
#else
    epoll_event event{EPOLLIN | (once ? EPOLLONESHOT : EPOLLET), socket};
    epoll_ctl(m_epoll, EPOLL_CTL_ADD, socket->descriptor(), &event);

    ++m_count;
#endif
}

void Epoll::erase(AbstractSocket * const socket)
{
#ifdef _WIN32
    // Win32
#else
    epoll_ctl(m_epoll, EPOLL_CTL_DEL, socket->descriptor(), nullptr);
    delete socket;

    --m_count;
#endif
}

void Epoll::epoll(int interval, std::vector<AbstractSocket *> &events)
{
#ifdef _WIN32
    // Remove invalid events
    if(!m_removeBuf.empty() && !m_events.empty())
    {
        for(auto it = m_events.begin(); it < m_events.end();)
        {
            if(m_removeBuf.find(it->fd) != m_removeBuf.end())
                it = m_events.erase(it);
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
    int ret = -1;
    if((ret = epoll_wait(m_epoll, m_eventBuf, MAX_EVENTS, interval)) <= 0)
        return;

    epoll_event *item = nullptr;
    AbstractSocket *socket = nullptr;
    for(int i = 0; i < ret; ++i)
    {
        item = m_eventBuf + i;
        socket = reinterpret_cast<AbstractSocket *>(item->data.ptr);

        if(item->events & EPOLLIN)
            events.emplace_back(socket);
        else if(item->events & EPOLLERR || item->events & EPOLLHUP)
        {
            socket->timer()->deleteLater();
            this->erase(socket);
        }
    }
#endif
}
