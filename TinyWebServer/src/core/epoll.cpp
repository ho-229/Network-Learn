/**
 * @author Ho 229
 * @date 2021/8/20
 */

#include "epoll.h"

#include <algorithm>

Epoll::Epoll()
{
#if defined (OS_WINDOWS)
    m_connections.reserve(512);
#elif defined (OS_LINUX)
    m_epoll = epoll_create1(0);
#else
    m_kqueue = kqueue();
#endif
}

Epoll::~Epoll()
{
#if defined (OS_LINUX)
    close(m_epoll);
#endif
}

void Epoll::insert(AbstractSocket * const socket, bool exclusive)
{
#if defined (OS_WINDOWS)
    static_cast<void>(exclusive);   // Unused

    std::unique_lock<std::mutex> lock(m_mutex);

    m_events.emplace_back(pollfd{socket->descriptor(), POLLIN, 0});

    m_connections.insert(ConnectionItem(socket->descriptor(), socket));
#else   // *nix
# if defined (OS_LINUX)     // Linux
    epoll_event event{(exclusive ? EPOLLEXCLUSIVE | EPOLLIN : EPOLLIN) | EPOLLET, socket};
    epoll_ctl(m_epoll, EPOLL_CTL_ADD, socket->descriptor(), &event);
# else                      // Unix
    struct kevent event;
    EV_SET(&event, socket->descriptor(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, socket);
    kevent(m_kqueue, &event, 1, nullptr, 0, nullptr);
# endif
    ++m_count;
#endif
}

void Epoll::erase(AbstractSocket * const socket)
{
#if defined (OS_WINDOWS)    // Windows
    std::unique_lock<std::mutex> lock(m_mutex);

    this->eraseEvent(socket);
    m_connections.erase(socket->descriptor());
#else   // *nix
# if defined (OS_LINUX)     // Linux
    epoll_ctl(m_epoll, EPOLL_CTL_DEL, socket->descriptor(), nullptr);
# else                      // Unix
    struct kevent event;
    EV_SET(&event, socket->descriptor(), EVFILT_READ, EV_DELETE | EV_DISABLE, 0, 0, socket);
    kevent(m_kqueue, &event, 1, nullptr, 0, nullptr);
# endif
    --m_count;
#endif
}

#if defined (OS_WINDOWS)
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
#if defined (OS_WINDOWS)    // Windows
    if(m_events.empty())
        return;

    m_mutex.lock();
    auto temp = m_events;
    m_mutex.unlock();

    if(WSAPoll(&temp[0], ULONG(temp.size()), EPOLL_WAIT_TIMEOUT) <= 0)
        return;

    for(const auto &item : temp)
    {
        const auto it = m_connections.find(item.fd);
        if(it == m_connections.end())
            continue;

        if(item.revents & POLLIN)
            events.emplace_back(it->second);
        else if(item.revents & POLLERR || item.revents & POLLHUP)
            errorEvents.emplace_back(it->second);
    }
#else   // *nix
# if defined (OS_LINUX)     // Linux
    int ret = -1;
    if((ret = epoll_wait(m_epoll, m_eventBuf, EPOLL_MAX_EVENTS, EPOLL_WAIT_TIMEOUT)) <= 0)
        return;

    epoll_event *item = nullptr;
    for(int i = 0; i < ret; ++i)
    {
        item = m_eventBuf + i;

        if(item->events & EPOLLIN)
            events.emplace_back(reinterpret_cast<AbstractSocket *>(item->data.ptr));
        else if(item->events & EPOLLERR || item->events & EPOLLHUP)
            errorEvents.emplace_back(reinterpret_cast<AbstractSocket *>(item->data.ptr));
    }
# else                      // Unix
    const timespec timeout{0, EPOLL_WAIT_TIMEOUT * 1000'000};
    int ret = -1;
    if((ret = kevent(m_kqueue, nullptr, 0, m_eventBuf, EPOLL_MAX_EVENTS, &timeout)) <= 0)
        return;

    struct kevent *item = nullptr;
    for(int i = 0; i < ret; ++i)
    {
        item = m_eventBuf + i;

        if(item->flags & EV_EOF || item->flags & EV_ERROR)
            errorEvents.emplace_back(reinterpret_cast<AbstractSocket *>(item->udata));
        else if(item->filter == EVFILT_READ)
            events.emplace_back(reinterpret_cast<AbstractSocket *>(item->udata));
    }
# endif
#endif
}
