/**
 * @author Ho 229
 * @date 2021/7/26
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "event.h"

#include <vector>
#include <string>
#include <memory>
#include <thread>

#define ANY_HOST "0.0.0.0"

class Epoll;
class Event;
class TcpSocket;
class HttpServices;

typedef std::pair<std::string, std::string> ServerPort;
typedef std::pair<std::thread, std::shared_ptr<Epoll>> EpollItem;

class WebServer
{
public:
    explicit WebServer();
    virtual ~WebServer();

    int exec();

    HttpServices *services() const { return m_services; }

    /**
     * @brief WebServer::exec() loop interval
     */
    void setInterval(long microSecond) { m_interval.second = microSecond * 1000; }
    long interval() const { return m_interval.second; }

    /**
     * @brief Keep alive timeout
     */
    void setTimeout(int microSecond) { m_timeout = microSecond; }
    int timeout() const { return m_timeout; }

    void setMaxTimes(int num) { m_maxTimes = num > 0 ? num : 30; }
    int maxTimes() const { return m_maxTimes; }

    void setThreadCount(size_t num) { m_threadCount = num; }
    size_t threadCount() const { return m_threadCount; }

    void listen(const std::string& hostName, const std::string& port,
                bool sslEnable = false);

    void quit() { m_runnable = false; }

    template <typename Func>
    void installEventHandler(const Func& handler) { m_handler = handler; }

private:
    bool session(AbstractSocket * const connect) const;

    bool m_isLoaded = true;
    bool m_runnable = true;
    int m_timeout = 30000;       // 30s
    int m_maxTimes = 30;
    size_t m_threadCount;

    std::pair<long, long> m_interval = {0, 500 * 1000};     // 500ms

    std::vector<EpollItem> m_epolls;

    std::vector<std::shared_ptr<TcpSocket>> m_listeners;

    HttpServices *m_services = nullptr;

    EventHandler m_handler = [](Event *){};
};

#endif // WEBSERVER_H
