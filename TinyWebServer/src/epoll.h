/**
 * @author Ho 229
 * @date 2021/8/20
 */

#ifndef EPOLL_H
#define EPOLL_H

#include "abstractsocket.h"
#include "timermanager.h"

#ifdef _WIN32
# include <WinSock2.h>
# define POLL(x, y, z) WSAPoll(x, y, z)
#else
# include <poll.h>
# include <sys/epoll.h>
# define POLL(x, y, z) poll(x, y, z)
#endif

#include <mutex>
#include <vector>
#include <memory>
#include <unordered_map>

typedef std::function<bool(AbstractSocket * const)> SessionHandler;

class Epoll
{
public:
    explicit Epoll();
    ~Epoll();

    void addConnection(AbstractSocket *socket);

    void exec(int interval, const SessionHandler& handler);

    void setMaxTimes(int max) { m_maxTimes = max > 0 ? max : 10; }
    int maxTimes() const { return m_maxTimes; }

    void setTimeout(int timeout) { m_timerManager.setTimeout(timeout); }
    int timeout() const { return m_timerManager.timeout(); }

private:
    inline void removeConnection(const Socket& socket);

    std::unordered_map<Socket, std::shared_ptr<AbstractSocket>> m_connections;

    std::mutex m_mutex;
    std::condition_variable m_condition;

    TimerManager<Socket> m_timerManager;

    int m_maxTimes = 10;
    bool m_runnable = true;

#ifdef _WIN32
    std::vector<pollfd> m_events;
#endif

    using Connection = decltype (m_connections)::value_type;
};

#endif // EPOLL_H
