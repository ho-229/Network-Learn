/**
 * @author Ho 229
 * @date 2021/8/20
 */

#ifndef EPOLL_H
#define EPOLL_H

#include "../define.h"
#include "../abstract/abstractsocket.h"

#include <vector>
#include <memory>
#include <functional>

#ifdef _WIN32
# include <unordered_map>
typedef std::vector<pollfd> EventList;
#else
# include <sys/epoll.h>
# include <unistd.h>
#endif

class Epoll
{
public:
    explicit Epoll();
    ~Epoll();

    void insert(AbstractSocket *const socket, bool once = false);
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
#ifdef _WIN32
    inline void eraseEvent(AbstractSocket *const socket);

    typedef std::pair<bool,                             // Once
                      std::unique_ptr<AbstractSocket>>  // Socket
        Connection;

    std::unordered_map<Socket, Connection> m_connections;

    using ConnectionItem = decltype (m_connections)::value_type;

    std::vector<pollfd> m_events;
#else
    int m_epoll = 0;
    size_t m_count = 0;
    epoll_event m_eventBuf[EPOLL_MAX_EVENTS];
#endif
};

#endif // EPOLL_H
