/**
 * @author Ho 229
 * @date 2021/8/6
 */

#ifndef ABSTRACTSOCKET_H
#define ABSTRACTSOCKET_H

#include <string>

#define BUF_SIZE 64
#define SOCKET_BUF_SIZE 4096

#define SOCKET_INFO_ENABLE 0

extern "C"
{
#ifdef _WIN32
typedef unsigned int Socket;
# define DEFAULT_SOCKET Socket(~0)

# include <WinSock2.h>
# include <WS2tcpip.h>
#else
typedef int Socket;
# define DEFAULT_SOCKET -1

# include <sys/socket.h>
#endif
}

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

#if SOCKET_INFO_ENABLE
    inline void initializeInfo()
    {
        sockaddr_in addr;
        socklen_t len = sizeof (addr);

        if(this->isListening())
            getsockname(m_descriptor, reinterpret_cast<sockaddr *>(&addr), &len);
        else
            getpeername(m_descriptor, reinterpret_cast<sockaddr *>(&addr), &len);

        char hostName[32];
        m_hostName = inet_ntop(addr.sin_family, &addr.sin_addr,
                               hostName, sizeof (hostName));
        m_port = std::to_string(ntohs(addr.sin_port));
    }


    std::string hostName() const { return m_hostName; }
    std::string port() const { return m_port; }
#endif

    Socket descriptor() const { return m_descriptor; }

    static inline constexpr bool isValid(const Socket& sock)
    {
#ifdef _WIN32
        return sock != Socket(~0);
#else
        return sock > 0;
#endif
    }

    void setTimer(Timer<AbstractSocket *> *timer) { m_timer = timer; }
    Timer<AbstractSocket *> *timer() const { return m_timer; }

    void addTimes() { ++m_times; }
    size_t times() const { return m_times; }

protected:
    explicit AbstractSocket(const Socket socket = DEFAULT_SOCKET) :
        m_descriptor(socket)
    {
#if SOCKET_INFO_ENABLE
        if(isValid(socket))
            this->initializeInfo();
#endif
    }

    // Disable copy
    AbstractSocket(AbstractSocket& other) = delete;
    AbstractSocket& operator=(const AbstractSocket& other) = delete;

    Socket m_descriptor = 0;

    size_t m_times = 0;

#if SOCKET_INFO_ENABLE
    std::string m_hostName;
    std::string m_port;
#endif

    Timer<AbstractSocket *> *m_timer = nullptr;
};

#endif // ABSTRACTSOCKET_H
