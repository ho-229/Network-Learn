/**
 * @author Ho 229
 * @date 2021/7/26
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <string>
#include <functional>

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

    void setSslEnable(bool enable = true);
    bool sslEnable() const { return m_sslEnable; }

    void setInterval(long microSecond) { m_timeout.second = microSecond * 1000; }
    long interval() const { return m_timeout.second; }

    void setPort(const ServerPort& port)
    {
        if(port.first == port.second)
            return;

        m_port = port;
    }

    ServerPort port() const { return m_port; }

    template <typename Func>
    void installEventHandler(const Func& handler) { m_handler = handler; }

private:
    void session(AbstractSocket * const connect);

    bool m_isLoaded = true;
    bool m_runnable = true;
    bool m_sslEnable = false;

    std::pair<long, long> m_timeout = {0, 500 * 1000};

    std::pair<TcpSocket *,      // HTTP
              TcpSocket *>      // HTTPS
        m_listenSockets = { nullptr, nullptr };

    HttpServices *m_services = nullptr;

    ServerPort m_port = {"80", "443"};

    EventHandler m_handler = [](Event *){};
};

#endif // WEBSERVER_H
