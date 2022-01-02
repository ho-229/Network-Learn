/**
 * @author Ho 229
 * @date 2021/8/6
 */

#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include "../abstract/abstractsocket.h"

class TcpSocket : public AbstractSocket
{
public:
    explicit TcpSocket(const Socket socket = INVALID_SOCKET);
    ~TcpSocket() override;

    void read(std::string& buffer) override;
    ssize_t write(const char* buf, size_t count) override;

    void close() override;

    bool sslEnable() const override { return m_sslEnable; }

    bool isValid() const override { return AbstractSocket::isValid(m_descriptor); }

    ssize_t sendFile(File file, off_t offset, size_t count) override;

    /**
     * @ref CS:APP(3) P662: int open_listenfd(char *port)
     * @brief Listen on the given port
     * @return true when successful
     */
    bool listen(const std::string& hostName, const std::string& port, bool sslEnable);
    bool isListener() const override { return m_isListener; }

    Socket accept() const;

#ifdef _WIN32
    static bool initializatWsa();
    static void cleanUpWsa();
#endif

private:
    bool m_isListener = false;
    bool m_sslEnable = false;
};

#endif // TCPSOCKET_H
