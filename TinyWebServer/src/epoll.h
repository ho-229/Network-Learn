/**
 * @author Ho 229
 * @date 2021/8/20
 */

#ifndef EPOLL_H
#define EPOLL_H

#include "abstractsocket.h"
#include "timermanager.h"
#include "event.h"

#ifdef _WIN32
# include <WinSock2.h>
# include <vector>
#else
# include <sys/epoll.h>
# include <unistd.h>

# define MAX_EVENTS 128
#endif

#include <mutex>
#include <memory>
#include <functional>
#include <unordered_map>
#include <condition_variable>

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

    size_t count() const { return m_connections.size(); }

    template <typename Func>
    void installEventHandler(const Func& handler) { m_handler = handler; }

private:
    inline void removeConnection(const Socket& socket);

    std::unordered_map<Socket, std::shared_ptr<AbstractSocket>> m_connections;

    std::mutex m_mutex;
    std::condition_variable m_condition;

    TimerManager<Socket> m_timerManager;

    EventHandler m_handler = {};

    int m_maxTimes = 20;
    bool m_runnable = true;

#ifdef _WIN32
    std::vector<pollfd> m_events;
#else
    int m_epoll = 0;
#endif

    using Connection = decltype (m_connections)::value_type;
};

#endif // EPOLL_H
