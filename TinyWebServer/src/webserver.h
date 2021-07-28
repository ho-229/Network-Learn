/**
 * @author Ho 229
 * @date 2021/7/26
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <string>
#include <functional>

class Event;
class HttpServices;

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

public:

    bool runnable = true;

private:
    /**
     * @ref CS:APP(3) P662: int open_listenfd(char *port)
     * @brief Listen on the given port
     * @return Listen fd, failed return -1
     */
    static int startListen(const std::string& port);

    bool m_isLoaded = true;

    HttpServices *m_services = nullptr;
    std::string m_port = "8080";

    EventHandler m_handler = [](Event *){};
};

#endif // WEBSERVER_H
