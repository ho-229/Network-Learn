/**
 * @author Ho 229
 * @date 2021/8/20
 */

#include "epoll.h"

#if defined (OS_UNIX) || defined (OS_LINUX)

Epoll::Epoll()
{
#if defined (OS_LINUX)
    m_epoll = epoll_create1(0);
#elif defined (OS_UNIX)
    m_kqueue = kqueue();
#endif
}

Epoll::~Epoll()
{
#if defined (OS_LINUX)
    close(m_epoll);
#elif defined (OS_UNIX)
    close(m_kqueue);
#endif
}

void Epoll::insert(AbstractSocket * const socket, bool exclusive)
{
#if defined (OS_LINUX)     // Linux
    epoll_event event{(exclusive ? EPOLLEXCLUSIVE | EPOLLIN : EPOLLIN | EPOLLRDHUP)
                | EPOLLET, socket};
    epoll_ctl(m_epoll, EPOLL_CTL_ADD, socket->descriptor(), &event);
#elif defined (OS_UNIX)                     // Unix
    struct kevent event;
    EV_SET(&event, socket->descriptor(), EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, socket);
    kevent(m_kqueue, &event, 1, nullptr, 0, nullptr);
# endif
    ++m_count;
}

void Epoll::erase(AbstractSocket * const socket)
{
#if defined (OS_LINUX)     // Linux
    epoll_ctl(m_epoll, EPOLL_CTL_DEL, socket->descriptor(), nullptr);
#elif defined (OS_UNIX)                      // Unix
    struct kevent event;
    EV_SET(&event, socket->descriptor(), EVFILT_READ, EV_DELETE | EV_DISABLE, 0, 0, socket);
    kevent(m_kqueue, &event, 1, nullptr, 0, nullptr);
# endif
    --m_count;
}

void Epoll::epoll(std::vector<AbstractSocket *> &events,
                  std::vector<AbstractSocket *> &errorEvents)
{
#if defined (OS_LINUX)     // Linux
    int ret = -1;
    if((ret = epoll_wait(m_epoll, m_eventBuf, EPOLL_MAX_EVENTS, EPOLL_WAIT_TIMEOUT)) <= 0)
        return;

    epoll_event *item = nullptr;
    for(int i = 0; i < ret; ++i)
    {
        item = m_eventBuf + i;

        if(item->events & EPOLLERR || item->events & EPOLLHUP || item->events & EPOLLRDHUP)
            errorEvents.emplace_back(reinterpret_cast<AbstractSocket *>(item->data.ptr));
        else if(item->events & EPOLLIN)
            events.emplace_back(reinterpret_cast<AbstractSocket *>(item->data.ptr));
    }
#elif defined (OS_UNIX)                      // Unix
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
#endif
}

size_t Epoll::count() const
{
    return m_count;
}

#endif
