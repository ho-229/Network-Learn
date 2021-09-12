/**
 * @author Ho 229
 * @date 2021/8/20
 */

#ifndef EPOLL_H
#define EPOLL_H

#include "abstractsocket.h"

#include <memory>
#include <vector>
#include <functional>
#include <unordered_set>

#ifdef _WIN32
# include <WinSock2.h>

# define ERROR_EVENT POLLERR
# define CLOSE_EVENT POLLHUP

typedef std::vector<pollfd> EventList;
#else
# include <sys/epoll.h>
# include <unistd.h>

# define ERROR_EVENT EPOLLERR
# define CLOSE_EVENT EPOLLHUP

# define MAX_EVENTS 128
typedef std::vector<epoll_event> EventList;
#endif

typedef std::function<void(const Socket, bool)> SessionHandler;

class Epoll
{
public:
    explicit Epoll();
    ~Epoll();

    void addConnection(const Socket socket);
    void removeConnection(const Socket socket);

    const EventList epoll(int interval);

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
#endif
};

#endif // EPOLL_H
