/**
 * @author Ho 229
 * @date 2021/8/20
 */

#ifndef EPOLL_H
#define EPOLL_H

#include "abstractsocket.h"

#ifdef _WIN32
# include <WinSock2.h>
# include <vector>
#else
# include <sys/epoll.h>
# include <unistd.h>

# define MAX_EVENTS 128
#endif

#include <memory>
#include <functional>
#include <unordered_map>

typedef std::function<void(const Socket, bool)> SessionHandler;

class Epoll
{
public:
    explicit Epoll();
    ~Epoll();

    void addConnection(const Socket socket);
    void removeConnection(const Socket socket);

    void process(int interval, const SessionHandler& handler);

    size_t count() const
    {
#ifdef _WIN32
        return m_events.size();
#endif
    }

private:
#ifdef _WIN32
    std::vector<Socket> m_removeBuf;
    std::vector<Socket> m_addBuf;
    std::vector<pollfd> m_events;
#else
    int m_epoll = 0;
    size_t m_count = 0;
#endif
};

#endif // EPOLL_H
