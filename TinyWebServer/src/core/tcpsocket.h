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

    ssize_t read(char *buf, size_t count) override;
    ssize_t write(const char* buf, size_t count) override;

    void close() override;

    bool sslEnable() const override { return false; }

    bool isValid() const override { return AbstractSocket::isValid(m_descriptor); }

    ssize_t sendFile(File file, off_t offset, size_t count) override;
};

#endif // TCPSOCKET_H
