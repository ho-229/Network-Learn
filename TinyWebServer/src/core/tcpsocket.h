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
    explicit TcpSocket(const Socket socket = DEFAULT_SOCKET);
    ~TcpSocket() override;

    void read(std::string& buffer) override;
    int write(const char* buf, int size) override;

    void close() override;

    bool sslEnable() const override { return m_sslEnable; }

    bool isValid() const override { return AbstractSocket::isValid(m_descriptor); }

    /**
     * @ref CS:APP(3) P662: int open_listenfd(char *port)
     * @brief Listen on the given port
     * @return true when successful
     */
    bool listen(const std::string& hostName, const std::string& port, bool sslEnable);
    bool isListening() const override { return m_isListening; }

    Socket accept() const;

#ifdef _WIN32
    static bool initializatWsa();
    static void cleanUpWsa();
#endif

private:
    bool m_isListening = false;
    bool m_sslEnable = false;
};

#endif // TCPSOCKET_H
