/**
 * @author Ho 229
 * @date 2021/7/26
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "event.h"
#include "timermanager.h"
#include "abstractsocket.h"

#include <mutex>
#include <vector>
#include <string>
#include <memory>
#include <thread>
#include <unordered_map>

#define ANY_HOST "0.0.0.0"

class Epoll;
class Event;
class TcpSocket;
class HttpServices;

typedef std::pair<std::string, std::string> ServerPort;

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
    void setInterval(int ms) { m_interval = ms; }
    int interval() const { return m_interval; }

    /**
     * @brief Keep alive timeout
     */
    void setTimeout(int ms) { m_timerManager.setTimeout(ms); }
    int timeout() const { return m_timerManager.timeout(); }

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
    std::unordered_map<Socket, std::shared_ptr<AbstractSocket>> m_connections;
    using Connection = decltype (m_connections)::value_type;

    void popConnection(const Socket socket);
    void readableHandler(const Socket socket, bool isAvailable);
    bool session(std::shared_ptr<AbstractSocket> connect);

    bool m_isLoaded = true;
    bool m_runnable = true;
    int m_maxTimes = 30;
    size_t m_threadCount;

    int m_interval = 500;       // 500ms

    std::shared_ptr<Epoll> m_epoll;
    std::mutex m_mutex;

    TimerManager<Socket> m_timerManager;

    HttpServices *m_services = nullptr;

    EventHandler m_handler = [](Event *){};
};

#endif // WEBSERVER_H
