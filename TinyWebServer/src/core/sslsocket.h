/**
 * @author Ho 229
 * @date 2021/8/7
 */

#ifndef SSLSOCKET_H
#define SSLSOCKET_H

#include "../abstract/abstractsocket.h"

#include <openssl/ssl.h>

class SslSocket : public AbstractSocket
{
public:
    explicit SslSocket(const Socket socket);
    ~SslSocket() override;

    void read(std::string& buffer) override;
    int write(const char* buf, int size) override;

    void close() override;

    bool sslEnable() const override { return true; }

    bool isValid() const override { return AbstractSocket::isValid(m_descriptor)
               && m_ssl; }

    bool isListening() const override { return false; }

#ifdef __linux__
    ssize_t sendFile(int fd, off_t offset, size_t count) override;
#endif

    static bool initializatSsl(const std::string& certFile,
                               const std::string& privateKey);
    static void cleanUpSsl();
    static std::string sslVersion();
    static bool isSslAvailable() { return sslContext; }

private:
    static SSL_CTX *sslContext;

    SSL *m_ssl = nullptr;
};

#endif // SSLSOCKET_H
