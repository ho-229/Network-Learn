/**
 * @author Ho 229
 * @date 2021/8/20
 */

#ifndef EPOLL_H
#define EPOLL_H

#include "../define.h"
#include "../abstract/abstractsocket.h"

#include <vector>

#if defined (OS_WINDOWS)
# include <mutex>
# include <unordered_map>
typedef std::vector<pollfd> EventList;
#else
# if defined (OS_LINUX)
#  include <sys/epoll.h>
# else
#  include <sys/event.h>
#  include <sys/types.h>
#  include <sys/time.h>
# endif
# include <unistd.h>
# include <atomic>
#endif

class Epoll
{
public:
    explicit Epoll();
    ~Epoll();

    void insert(AbstractSocket *const socket, bool exclusive = false);
    void erase(AbstractSocket *const socket);

    void epoll(std::vector<AbstractSocket *> &events,
               std::vector<AbstractSocket *> &errorEvents);

    size_t count() const
    {
#ifdef _WIN32
        return m_events.size();
#else
        return m_count;
#endif
    }

private:
#if defined (OS_WINDOWS)    // Windows poll
    inline void eraseEvent(AbstractSocket *const socket);

    std::unordered_map<Socket, AbstractSocket *> m_connections;

    using ConnectionItem = decltype (m_connections)::value_type;

    std::vector<pollfd> m_events;
    std::mutex m_mutex;
#else
    std::atomic_uint m_count = 0;
# if defined (OS_LINUX)     // Linux epoll
    int m_epoll = 0;
    epoll_event m_eventBuf[EPOLL_MAX_EVENTS];
# else                      // Unix kqueue
    int m_kqueue = 0;
    std::vector<struct kevent> m_changes;
    struct kevent m_eventBuf[EPOLL_MAX_EVENTS];
# endif
#endif
};

#endif // EPOLL_H
