/**
 * @author Ho 229
 * @date 2021/10/3
 */

#ifndef CONNECTIONPOOL_H
#define CONNECTIONPOOL_H

#include "event.h"
#include "epoll.h"
#include "timermanager.h"
#include "abstractsocket.h"

#include <unordered_map>

class HttpServices;

class ConnectionPool
{
public:
    explicit ConnectionPool();
    ~ConnectionPool();

    void setMaxTimes(int num) { m_maxTimes = num > 0 ? num : 30; }
    int maxTimes() const { return m_maxTimes; }

    void setServices(HttpServices *services) { m_services = services; }
    HttpServices *services() const { return m_services; }

    /**
     * @brief Keep alive timeout
     */
    void setTimeout(int ms) { m_manager.setTimeout(ms); }
    int timeout() const { return m_manager.timeout(); }

    template <typename Func>
    void installEventHandler(const Func& handler) { m_handler = handler; }

    void addConnection(std::shared_ptr<AbstractSocket> socket);

    void exec(int interval);

    void quit() { m_runnable = false; }

private:
    bool session(AbstractSocket * const socket);
    void eventsHandler(const EventList& events);
    inline void release(const AbstractSocket *socket);

    static bool sendFile(std::ifstream& stream, AbstractSocket *socket);

    int m_maxTimes = 30;

    std::unordered_map<Socket, std::shared_ptr<AbstractSocket>> m_connections;
    using Connection = decltype (m_connections)::value_type;

    std::shared_ptr<Epoll> m_epoll;

    std::atomic_bool m_runnable = false;

    TimerManager<Socket> m_manager;

    HttpServices *m_services = nullptr;

    EventHandler m_handler;
};

#endif // CONNECTIONPOOL_H
