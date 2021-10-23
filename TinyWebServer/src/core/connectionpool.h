/**
 * @author Ho 229
 * @date 2021/10/3
 */

#ifndef CONNECTIONPOOL_H
#define CONNECTIONPOOL_H

#include "epoll.h"
#include "../until/event.h"
#include "../until/timermanager.h"
#include "../abstract/abstractsocket.h"

#include <atomic>
#include <thread>
#include <unordered_map>

class AbstractServices;

class ConnectionPool
{
public:
    explicit ConnectionPool(const std::atomic_bool &runnable,
                            int timeout, int interval,
                            AbstractServices *const services,
                            const EventHandler &handler);
    ~ConnectionPool();

    size_t count() const { return m_connections.size(); }

    void addConnection(std::shared_ptr<AbstractSocket> socket);

    std::thread &thread() { return m_thread; }

protected:
    void exec(int interval);

private:
    void eventsHandler(const EventList& events);
    inline void release(const AbstractSocket *socket);

    std::unordered_map<Socket, std::shared_ptr<AbstractSocket>> m_connections;
    using Connection = decltype (m_connections)::value_type;

    Epoll m_epoll;

    const std::atomic_bool &m_runnable;

    TimerManager<Socket> m_manager;

    AbstractServices *const m_services;

    EventHandler m_handler;

    std::thread m_thread;
};

#endif // CONNECTIONPOOL_H
