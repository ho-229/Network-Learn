/**
 * @author Ho 229
 * @date 2021/8/20
 */

#include "epoll.h"

#include <algorithm>

Epoll::Epoll()
{
#ifdef _WIN32
    m_connections.reserve(512);
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
    m_events.emplace_back(pollfd{socket->descriptor(), POLLIN, 0});

    m_connections.insert(ConnectionItem(socket->descriptor(),
                                        Connection(once, socket)));
#else
    epoll_event event{EPOLLIN | (once ? EPOLLONESHOT : EPOLLET), socket};
    epoll_ctl(m_epoll, EPOLL_CTL_ADD, socket->descriptor(), &event);

    ++m_count;
#endif
}

void Epoll::erase(AbstractSocket * const socket)
{
#ifdef _WIN32
    this->eraseEvent(socket);
    m_connections.erase(socket->descriptor());

    delete socket;
#else
    epoll_ctl(m_epoll, EPOLL_CTL_DEL, socket->descriptor(), nullptr);
    delete socket;

    --m_count;
#endif
}

#ifdef _WIN32
void Epoll::eraseEvent(AbstractSocket * const socket)
{
    const auto it = std::find_if(m_events.cbegin(), m_events.cend(),
                                 [socket](const pollfd &event) -> bool {
                                     return socket->descriptor() == event.fd;
                                 });
    if(it != m_events.cend())
        m_events.erase(it);
}
#endif

void Epoll::epoll(std::vector<AbstractSocket *> &events,
                  std::vector<AbstractSocket *> &errorEvents)
{
#ifdef _WIN32
    if(m_events.empty())
        return;

    auto temp = m_events;
    if(WSAPoll(&temp[0], ULONG(temp.size()), EPOLL_WAIT_TIMEOUT) <= 0)
        return;

    for(const auto &item : temp)
    {
        const auto it = m_connections.find(item.fd);
        if(it == m_connections.end())
            continue;

        if(item.revents & POLLIN)
        {
            events.emplace_back(it->second.second);

            if(it->second.first)
                this->eraseEvent(it->second.second);
        }
        else if(item.revents & POLLERR || item.revents & POLLHUP)
            errorEvents.emplace_back(it->second.second);
    }
#else   // Unix
    int ret = -1;
    if((ret = epoll_wait(m_epoll, m_eventBuf, EPOLL_MAX_EVENTS, EPOLL_WAIT_TIMEOUT)) <= 0)
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
            errorEvents.emplace_back(socket);
    }
#endif
}
