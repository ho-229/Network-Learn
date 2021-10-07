/**
 * @author Ho 229
 * @date 2021/7/26
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "until/event.h"
#include "until/threadpool.h"
#include "core/connectionpool.h"
#include "abstract/abstractservices.h"

#include <string>

#define ANY_HOST "0.0.0.0"

class AbstractSocket;
class AbstractServices;

typedef std::pair<std::string, std::string> ServerPort;

class WebServer
{
public:
    explicit WebServer();
    virtual ~WebServer();

    int exec();

    void setServices(AbstractServices *services) { m_services.reset(services); }
    AbstractServices *services() const { return m_services.get(); }

    /**
     * @brief WebServer::exec() loop interval
     */
    void setInterval(int ms) { m_interval = ms; }
    int interval() const { return m_interval; }

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

    void quit() { m_runnable = false; }

    template <typename Func>
    void installEventHandler(const Func& handler) { m_handler = handler; }

private:
    bool m_isLoaded = true;
    bool m_runnable = true;

    size_t m_loopCount = std::thread::hardware_concurrency();

    int m_timeout = 30000;
    int m_interval = 500;       // 500ms

    //ThreadPool m_pool;

    std::vector<std::shared_ptr<ConnectionPool>> m_pools;
    std::vector<std::shared_ptr<AbstractSocket>> m_listeners;

    std::shared_ptr<AbstractServices> m_services;

    EventHandler m_handler = [](Event *){};
};

#endif // WEBSERVER_H
