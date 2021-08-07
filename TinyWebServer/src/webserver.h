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

class WebServer
{
public:
    explicit WebServer();
    virtual ~WebServer();

    void setPort(const std::string& port) { m_port = port; }
    std::string port() const { return m_port; }

    int exec();

    HttpServices *services() const { return m_services; }

    template <typename Func>
    void installEventHandler(const Func& handler) { m_handler = handler; }

private:
    void session(AbstractSocket *connect);

    bool m_isLoaded = true;
    bool m_runnable = true;

    TcpSocket *m_listenSocket = nullptr;

    HttpServices *m_services = nullptr;
    std::string m_port = "8080";

    EventHandler m_handler = [](Event *){};
};

#endif // WEBSERVER_H
