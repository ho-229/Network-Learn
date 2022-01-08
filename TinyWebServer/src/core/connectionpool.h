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
    explicit ConnectionPool(const volatile bool &runnable,
                            const std::chrono::milliseconds &timeout,
                            AbstractServices *const services,
                            const EventHandler &handler);
    ~ConnectionPool();

    size_t count() const { return m_epoll.count(); }

    template <typename Iter>
    void registerListeners(const Iter begin, const Iter end)
    {
        for(auto it = begin; it < end; ++it)
            m_epoll.insert(it->get());
    }

    template <typename Iter>
    void unregisterListeners(const Iter begin, const Iter end)
    {
        for(auto it = begin; it < end; ++it)
            (*it)->close();
    }

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
    void processQueue();
    void processErrorQueue();
    void processTimeoutQueue();

    Epoll m_epoll;

    std::vector<AbstractSocket *> m_queue;
    std::vector<AbstractSocket *> m_errorQueue;

    const volatile bool &m_runnable;
    const std::chrono::milliseconds &m_timeout;

    TimerManager<AbstractSocket *> m_manager;

    AbstractServices *const m_services;

    EventHandler m_handler;

    std::thread m_thread;
};

#endif // CONNECTIONPOOL_H
