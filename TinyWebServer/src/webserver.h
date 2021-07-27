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

public:
    std::string port = "8080";
    bool runnable = true;
    HttpServices *services = nullptr;

private:
    /**
     * @ref CS:APP(3) P662: int open_listenfd(char *port)
     * @brief Listen port
     * @return Listen fd
     */
    static int startListen(const std::string& port);

};

#endif // WEBSERVER_H
