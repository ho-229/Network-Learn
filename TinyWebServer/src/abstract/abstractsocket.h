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
# include <unistd.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
#endif
}

class AbstractSocket
{
public:
    virtual ~AbstractSocket() = default;

    virtual ssize_t read(char *buf, size_t count) = 0;
    virtual ssize_t write(const char* buf, size_t count) = 0;

    void read(std::string &buffer);

    template <typename String>
    inline ssize_t write(const String& data)
    { return this->write(data.c_str(), data.size()); }

    virtual void close() = 0;

    virtual bool sslEnable() const = 0;

    virtual bool isValid() const = 0;

    bool isListener() const { return m_isListener; }

#if SOCKET_INFO_ENABLE
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

    virtual ssize_t sendFile(File file, off_t offset, size_t count) = 0;

    /**
     * @ref CS:APP(3) P662: int open_listenfd(char *port)
     * @brief Listen on the given port
     * @return true when successful
     */
    bool listen(const std::string& hostName, const std::string& port);

    Socket accept() const;

#ifdef _WIN32
    static bool initializatWsa();
    static void cleanUpWsa();
#endif

    static inline constexpr bool isValid(const Socket& socket)
    {
#ifdef _WIN32
        return socket != Socket(~0);
#else
        return socket > 0;
#endif
    }

    static inline void close(Socket& socket)
    {
        Socket descriptor = INVALID_SOCKET;
        std::swap(descriptor, socket);

#ifdef _WIN32
        closesocket(descriptor);
#else
        ::close(descriptor);
#endif
    }

protected:
    explicit AbstractSocket(const Socket socket = INVALID_SOCKET);

    // Disable copy
    AbstractSocket(AbstractSocket& other) = delete;
    AbstractSocket& operator=(const AbstractSocket& other) = delete;

    Socket m_descriptor = 0;

    size_t m_times = 0;

    void *m_timer = nullptr;

    bool m_isListener = false;

#if SOCKET_INFO_ENABLE
    std::string m_hostName;
    std::string m_port;
#endif
};

#endif // ABSTRACTSOCKET_H
