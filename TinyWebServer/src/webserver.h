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

#define ANY_HOST "0.0.0.0"

class Epoll;
class Event;
class TcpSocket;
class HttpServices;
class AbstractSocket;

typedef std::pair<std::string, std::string> ServerPort;

class WebServer
{
public:
    explicit WebServer();
    virtual ~WebServer();

    int exec();

    HttpServices *services() const { return m_services; }

    void setInterval(long microSecond) { m_interval.second = microSecond * 1000; }
    long interval() const { return m_interval.second; }

    /**
     * @brief Keep alive timeout
     */
    void setTimeout(int microSecond) { m_timeout = microSecond; }
    int timeout() const { return m_timeout; }

    void setMaxTimes(int num) { m_maxTimes = num > 0 ? num : 30; }
    int maxTimes() const { return m_maxTimes; }

    void listen(const std::string& hostName, const std::string& port,
                bool sslEnable = false);

    template <typename Func>
    void installEventHandler(const Func& handler) { m_handler = handler; }

private:
    bool session(AbstractSocket * const connect) const;

    bool m_isLoaded = true;
    bool m_runnable = true;
    int m_timeout = 30000;       // 30s
    int m_maxTimes = 30;

    std::pair<long, long> m_interval = {0, 500 * 1000};

    std::shared_ptr<Epoll> m_epoll;

    std::vector<std::pair<std::shared_ptr<TcpSocket>, bool>> m_listeners;

    HttpServices *m_services = nullptr;

    EventHandler m_handler = [](Event *){};
};

#endif // WEBSERVER_H
