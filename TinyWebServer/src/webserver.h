/**
 * @author Ho 229
 * @date 2021/7/26
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <string>

class HttpServices;

class WebServer
{
public:
    explicit WebServer();
    virtual ~WebServer();

#ifdef _WIN32
    bool initialize();
#endif

    int exec();

    HttpServices *services() const { return m_services; }

public:
    std::string port = "8080";
    bool runnable = true;

private:
    /**
     * @ref CS:APP(3) P662: int open_listenfd(char *port)
     * @brief Listen port
     * @return Listen fd
     */
    static int startListen(const std::string& port);

    HttpServices *m_services = nullptr;

};

#endif // WEBSERVER_H
