/**
 * @author Ho 229
 * @date 2021/10/3
 */

#ifndef CONNECTIONPOOL_H
#define CONNECTIONPOOL_H

#include "epoll.h"
#include "../util/event.h"
#include "../util/timermanager.h"
#include "../abstract/abstractsocket.h"

#include <atomic>
#include <thread>

class AbstractServices;

class ConnectionPool
{
public:
    explicit ConnectionPool(const std::atomic_bool &runnable,
                            int timeout,
                            AbstractServices *const services,
                            const EventHandler &handler);
    ~ConnectionPool();

    size_t count() const { return m_epoll.count(); }

    void registerListener(AbstractSocket *const socket)
    { m_epoll.insert(socket); }

    inline void start()
    { m_thread = std::thread(std::bind(&ConnectionPool::exec, this)); }

    inline void waitForFinished()
    {
        if(m_thread.joinable())
            m_thread.join();
    }

protected:
    void exec();

private:
    void eventsHandler();

    Epoll m_epoll;

    std::vector<AbstractSocket *> m_queue;
    std::vector<AbstractSocket *> m_errorQueue;

    const std::atomic_bool &m_runnable;

    TimerManager<AbstractSocket *> m_manager;

    AbstractServices *const m_services;

    EventHandler m_handler;

    std::thread m_thread;
};

#endif // CONNECTIONPOOL_H
