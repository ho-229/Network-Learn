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
    explicit TcpSocket(const SocketInfo &info = {});
    ~TcpSocket() override;

    void read(std::string& buffer) override;
    int write(const char* buf, size_t size) override;

    void close() override;

    /**
     * @ref CS:APP(3) P662: int open_listenfd(char *port)
     * @brief Listen on the given port
     * @return true when successful
     */
    bool listen(const std::string& port);
    bool isListening() const { return m_isListening; }

    SocketInfo waitForAccept() const;

#ifdef _WIN32
    static bool initializatWsa();
    static void cleanUpWsa();
#endif

private:
    bool m_isListening = false;
};

#endif // TCPSOCKET_H
