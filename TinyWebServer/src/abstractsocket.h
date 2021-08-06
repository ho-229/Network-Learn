/**
 * @author Ho 229
 * @date 2021/8/6
 */

#ifndef ABSTRACTSOCKET_H
#define ABSTRACTSOCKET_H

#include <string>

#ifdef _WIN32
typedef unsigned int Socket;
#else
typedef int Socket;
#endif

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

protected:
    explicit AbstractSocket(Socket descriptor) : m_descriptor(descriptor) {}

    // Disable copy
    AbstractSocket(AbstractSocket& other) = delete;
    AbstractSocket& operator=(const AbstractSocket& other) = delete;

    Socket m_descriptor = 0;

    std::string m_hostName;
    std::string m_port;
};

#endif // ABSTRACTSOCKET_H
