/**
 * @author Ho 229
 * @date 2021/7/26
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <vector>
#include <string>
#include <memory>
#include <functional>

#define ANY_HOST "0.0.0.0"

class Event;
class TcpSocket;
class HttpServices;
class AbstractSocket;

typedef std::function<void(Event *)> EventHandler;
typedef std::pair<std::string, std::string> ServerPort;

class WebServer
{
public:
    explicit WebServer();
    virtual ~WebServer();

    int exec();

    HttpServices *services() const { return m_services; }

    void setInterval(uint32_t microSecond) { m_interval.second = long(microSecond) * 1000; }
    long interval() const { return m_interval.second; }

    void setTimeout(int microSecond) { m_timeout = microSecond; }
    int timeout() const { return m_timeout; }

    void listen(const std::string& hostName, const std::string& port, bool sslEnable = false);

    template <typename Func>
    void installEventHandler(const Func& handler) { m_handler = handler; }

private:
    void session(AbstractSocket * const connect);

    bool m_isLoaded = true;
    bool m_runnable = true;
    int m_timeout = 3000;       // in ms

    std::pair<long, long> m_interval = {0, 500 * 1000};

    std::vector<std::pair<std::shared_ptr<TcpSocket>, bool>> m_listeners;

    HttpServices *m_services = nullptr;

    EventHandler m_handler = [](Event *){};
};

#endif // WEBSERVER_H
