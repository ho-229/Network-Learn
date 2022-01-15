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

using namespace std::chrono_literals;

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
    void setTimeout(const std::chrono::milliseconds &ms) { m_timeout = ms; }
    std::chrono::milliseconds timeout() const { return m_timeout; }

    void setLoopCount(size_t count)
    { m_loopCount = count > 0 ? count : std::thread::hardware_concurrency(); }
    size_t loopCount() const { return m_loopCount; }

    /**
     * @note should run before WebServer::exec
     */
    void listen(const std::string& hostName, const std::string& port,
                bool sslEnable = false, std::pair<int, int> linger = {false, 0});

    int start();
    void quit() { m_runnable = false; }
    void requestQuit();

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
    volatile bool m_runnable = true;

    size_t m_loopCount = std::thread::hardware_concurrency();

    std::chrono::milliseconds m_timeout = 30s;

    std::vector<std::unique_ptr<ConnectionPool>> m_pools;
    std::vector<std::unique_ptr<AbstractSocket>> m_listeners;

    std::unique_ptr<AbstractServices> m_services;

    EventHandler m_handler = [](Event *){};
};

#endif // WEBSERVER_H
