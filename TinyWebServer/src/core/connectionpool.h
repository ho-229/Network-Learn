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

class AbstractServices;

class ConnectionPool
{
public:
    explicit ConnectionPool(const std::atomic_bool &runnable,
                            int timeout, int interval,
                            AbstractServices *const services,
                            const EventHandler &handler);
    ~ConnectionPool();

    size_t count() const { return m_epoll.count(); }

    void registerListener(AbstractSocket *const socket)
    { m_epoll.insert(socket); }

    std::thread &thread() { return m_thread; }

protected:
    void exec(int interval);

private:
    void eventsHandler();

    Epoll m_epoll;

    std::vector<AbstractSocket *> m_queue;

    const std::atomic_bool &m_runnable;

    TimerManager<AbstractSocket *> m_manager;

    AbstractServices *const m_services;

    EventHandler m_handler;

    std::thread m_thread;
};

#endif // CONNECTIONPOOL_H
