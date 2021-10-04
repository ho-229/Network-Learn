/**
 * @author Ho 229
 * @date 2021/8/6
 */

#ifndef ABSTRACTSOCKET_H
#define ABSTRACTSOCKET_H

#include <string>

#define BUF_SIZE 64
#define SOCKET_BUF_SIZE 4096

#ifdef _WIN32
typedef unsigned int Socket;
#else
typedef int Socket;
#endif

struct SocketInfo
{
    Socket descriptor;
    std::string hostName;
    std::string port;
};

template <typename T>
class Timer;

class AbstractSocket
{
public:
    virtual ~AbstractSocket() = default;

    virtual void read(std::string& buffer) = 0;
    virtual int write(const char* buf, int size) = 0;

    inline int write(const std::string& data)
    { return this->write(data.c_str(), int(data.size())); }

    virtual void close() = 0;

    virtual bool sslEnable() const = 0;

    virtual bool isValid() const = 0;

    virtual bool isListening() const = 0;

    std::string hostName() const { return m_hostName; }
    std::string port() const { return m_port; }

    Socket descriptor() const { return m_descriptor; }

    static inline constexpr bool isValid(const Socket& sock)
    {
#ifdef _WIN32
        return sock != Socket(~0);
#else
        return sock > 0;
#endif
    }

    void setTimer(Timer<Socket> *timer) { m_timer = timer; }
    Timer<Socket>* timer() const { return m_timer; }

    void addTimes() { ++m_times; }
    int times() const { return m_times; }

protected:
    explicit AbstractSocket(const SocketInfo& info = {}) :
        m_descriptor(info.descriptor),
        m_hostName(info.hostName),
        m_port(info.port)
    {}

    // Disable copy
    AbstractSocket(AbstractSocket& other) = delete;
    AbstractSocket& operator=(const AbstractSocket& other) = delete;

    Socket m_descriptor = 0;

    int m_times = 0;

    std::string m_hostName;
    std::string m_port;

    Timer<Socket> *m_timer = nullptr;
};

#endif // ABSTRACTSOCKET_H
