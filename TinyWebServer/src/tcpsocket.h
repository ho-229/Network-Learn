/**
 * @author Ho 229
 * @date 2021/8/6
 */

#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include "abstractsocket.h"

class TcpSocket : public AbstractSocket
{
public:
    explicit TcpSocket(Socket descriptor = 0);
    ~TcpSocket() override;

    void read(std::string& buffer) override;
    int write(const char* buf, size_t size) override;

    void close() override;

    bool listen(const std::string& port);
    bool isListening() const { return m_isListening; }

    TcpSocket* waitForAccept();

#ifdef _WIN32
    static bool initializatWsa();
    static void cleanUpWsa();
#endif

private:
    bool m_isListening = false;
};

#endif // TCPSOCKET_H
