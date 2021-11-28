/**
 * @author Ho 229
 * @date 2021/7/26
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "util/event.h"
#include "abstract/abstractservices.h"

#include <atomic>
#include <thread>
#include <memory>

#define ANY_HOST "0.0.0.0"

class ConnectionPool;
class AbstractSocket;
class AbstractServices;

class WebServer
{
public:
    explicit WebServer();
    virtual ~WebServer();

    void setServices(AbstractServices *services) { m_services.reset(services); }
    AbstractServices *services() const { return m_services.get(); }

    /**
     * @brief Keep alive timeout
     */
    void setTimeout(int ms) { m_timeout = ms; }
    int timeout() const { return m_timeout; }

    void setLoopCount(size_t count)
    { m_loopCount = count > 0 ? count : std::thread::hardware_concurrency(); }
    size_t loopCount() const { return m_loopCount; }

    /**
     * @note should run before WebServer::exec
     */
    void listen(const std::string& hostName, const std::string& port,
                bool sslEnable = false);

    int start();
    void quit() { m_runnable = false; }
    void waitForFinished();

    inline int exec()
    {
        int ret = 0;
        if((ret = this->start()) == 0)
            this->waitForFinished();

        return ret;
    }

    template <typename Func>
    void installEventHandler(const Func& handler) { m_handler = handler; }

private:
    bool m_isLoaded = true;
    std::atomic_bool m_runnable = true;

    size_t m_loopCount = std::thread::hardware_concurrency();

    int m_timeout = 30000;

    std::vector<std::shared_ptr<ConnectionPool>> m_pools;
    std::vector<std::shared_ptr<AbstractSocket>> m_listeners;

    std::shared_ptr<AbstractServices> m_services;

    EventHandler m_handler = [](Event *){};
};

#endif // WEBSERVER_H
