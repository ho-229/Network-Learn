﻿/**
 * @author Ho 229
 * @date 2021/8/20
 */

#include "epoll.h"
#include "../until/timermanager.h"

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

    m_connections[socket->descriptor()]
        = {once, std::shared_ptr<AbstractSocket>(socket)};
#else
    epoll_event event{EPOLLIN | (once ? EPOLLONESHOT : EPOLLET), socket};
    epoll_ctl(m_epoll, EPOLL_CTL_ADD, socket->descriptor(), &event);

    ++m_count;
#endif
}

void Epoll::erase(AbstractSocket * const socket)
{
#ifdef _WIN32
    auto const it = std::find_if(m_events.cbegin(), m_events.cend(),
                                 [socket](pollfd event) -> bool {
                                     return socket->descriptor() == event.fd;
                                 });
    if(it != m_events.cend())
        m_events.erase(it);

    m_connections.erase(socket->descriptor());
#else
    epoll_ctl(m_epoll, EPOLL_CTL_DEL, socket->descriptor(), nullptr);
    delete socket;

    --m_count;
#endif
}

void Epoll::epoll(int interval, std::vector<AbstractSocket *> &events)
{
#ifdef _WIN32
    if(m_events.empty())
    {
        Sleep(200);
        return;
    }

    auto temp = m_events;
    if(WSAPoll(&temp[0], ULONG(temp.size()), interval) <= 0)
        return;

    for(const auto &item : temp)
    {
        const auto it = m_connections.find(item.fd);
        if(it == m_connections.end())
            continue;

        if(item.revents & POLLIN)
            events.emplace_back(it->second.second.get());
        else if(item.revents & POLLERR || item.revents & POLLHUP)
        {
            it->second.second->timer()->deleteLater();
            this->erase(it->second.second.get());
        }
    }
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