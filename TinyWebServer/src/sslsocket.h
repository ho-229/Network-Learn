/**
 * @author Ho 229
 * @date 2021/8/7
 */

#ifndef SSLSOCKET_H
#define SSLSOCKET_H

#include "abstractsocket.h"

#include <openssl/ssl.h>

class SslSocket : public AbstractSocket
{
public:
    explicit SslSocket(const SocketInfo& info);
    ~SslSocket() override;

    void read(std::string& buffer) override;
    int write(const char* buf, int size) override;

    void close() override;

    bool sslEnable() const override { return m_ssl; }

    static bool initializatSsl(const std::string& certFile,
                               const std::string& privateKey);
    static bool isSslAvailable() { return sslContext; }
    static std::string sslVersion();
    static void cleanUpSsl();

private:
    static SSL_CTX *sslContext;

    SSL *m_ssl = nullptr;
};

#endif // SSLSOCKET_H
