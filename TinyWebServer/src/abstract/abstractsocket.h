/**
 * @author Ho 229
 * @date 2021/8/6
 */

#ifndef ABSTRACTSOCKET_H
#define ABSTRACTSOCKET_H

#include <string>
#include <memory>
#include <istream>

#include "../define.h"

extern "C"
{
#ifdef _WIN32
# include <WinSock2.h>
# include <WS2tcpip.h>
#else
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
#endif
}

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

    virtual bool isListener() const = 0;

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

    template <typename T>
    int setOption(int level, int option, const T value)
    {
        return setsockopt(m_descriptor, level, option,
                          reinterpret_cast<const char *>(&value), sizeof (T));
    }

    template <typename T>
    T option(int level, int option, int *ret = nullptr) const
    {
        T value;
        socklen_t len = sizeof (T);

        if(ret)
            *ret = getsockopt(m_descriptor, level, option, &value, &len);
        else
            getsockopt(m_descriptor, level, option, &value, &len);

        return value;
    }

    void setTimer(void *timer) { m_timer = timer; }
    void *timer() const { return m_timer; }

    void addTimes() { ++m_times; }
    size_t times() const { return m_times; }

    Socket descriptor() const { return m_descriptor; }

    bool sendStream(std::istream *const stream, size_t count = 0);

#ifdef __linux__
    virtual ssize_t sendFile(int fd, off_t offset, size_t count) = 0;
#endif

    static inline constexpr bool isValid(const Socket& sock)
    {
#ifdef _WIN32
        return sock != Socket(~0);
#else
        return sock > 0;
#endif
    }

protected:
    explicit AbstractSocket(const Socket socket = INVALID_SOCKET) :
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

    void *m_timer = nullptr;

    static thread_local std::unique_ptr<char[]> buffer;
};

#endif // ABSTRACTSOCKET_H
