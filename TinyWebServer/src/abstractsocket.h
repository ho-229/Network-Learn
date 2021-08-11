/**
 * @author Ho 229
 * @date 2021/8/6
 */

#ifndef ABSTRACTSOCKET_H
#define ABSTRACTSOCKET_H

#include <string>
#include <tuple>

#define BUF_SIZE 64
#define SOCKET_BUF_SIZE 4096

#ifdef _WIN32
typedef unsigned int Socket;
#else
typedef int Socket;
#endif

typedef std::tuple<Socket, std::string, std::string> SocketInfo;

class AbstractSocket
{
public:
    virtual ~AbstractSocket() = default;

    virtual void read(std::string& buffer) = 0;
    virtual int write(const char* buf, size_t size) = 0;

    inline int write(const std::string& data)
    { return this->write(data.c_str(), data.size()); }

    virtual void close() = 0;

    std::string hostName() const { return m_hostName; }
    std::string port() const { return m_port; }

    Socket descriptor() const { return m_descriptor; }

    constexpr inline static bool isValid(const Socket& sock)
    {
#ifdef _WIN32
        return sock != Socket(~0);
#else
        return sock > 0;
#endif
    }

    constexpr inline bool operator==(const Socket& socket) const
    { return this->m_descriptor == socket; }

    constexpr inline bool operator<(const AbstractSocket& other) const
    { return this->m_descriptor < other.m_descriptor; }

protected:
    explicit AbstractSocket(const SocketInfo& info = {}) :
        m_descriptor(std::get<0>(info)),
        m_hostName(std::get<1>(info)),
        m_port(std::get<2>(info))
    {}

    // Disable copy
    AbstractSocket(AbstractSocket& other) = delete;
    AbstractSocket& operator=(const AbstractSocket& other) = delete;

    Socket m_descriptor = 0;

    std::string m_hostName;
    std::string m_port;
};

#endif // ABSTRACTSOCKET_H
