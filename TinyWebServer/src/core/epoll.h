/**
 * @author Ho 229
 * @date 2021/8/20
 */

#ifndef EPOLL_H
#define EPOLL_H

#include "../abstract/abstractsocket.h"

#include <vector>
#include <memory>

#ifdef _WIN32
# include <unordered_set>
typedef std::vector<pollfd> EventList;
#else
# include <sys/epoll.h>
# include <unistd.h>

# define MAX_EVENTS 256
#endif

class Epoll
{
public:
    explicit Epoll();
    ~Epoll();

    void insert(AbstractSocket *const socket, bool once = false);
    void erase(AbstractSocket *const socket);

    void epoll(int interval, std::vector<AbstractSocket *> &events);

    size_t count() const
    {
#ifdef _WIN32
        return m_events.size();
#else
        return m_count;     // It maybe not correct
#endif
    }

private:
#ifdef _WIN32
    std::unordered_set<Socket> m_removeBuf;
    std::vector<pollfd> m_events;
#else
    int m_epoll = 0;
    size_t m_count = 0;
    epoll_event m_eventBuf[MAX_EVENTS];
#endif
};

#endif // EPOLL_H
