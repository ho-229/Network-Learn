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
    ssize_t write(const char* buf, size_t count) override;

    void close() override;

    bool sslEnable() const override { return true; }

    bool isValid() const override { return AbstractSocket::isValid(m_descriptor)
               && m_ssl; }

    bool isListener() const override { return false; }

    ssize_t sendFile(File file, off_t offset, size_t count) override;

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
