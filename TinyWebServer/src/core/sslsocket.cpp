/**
 * @author Ho 229
 * @date 2021/8/7
 */

#include "sslsocket.h"

#include <openssl/err.h>

#include <memory>

extern "C"
{
#ifdef _WIN32
# define CLOSE(x) closesocket(x)
#else   // Unix
# include <netdb.h>
# include <unistd.h>
# include <string.h>
# include <sys/mman.h>

# define CLOSE(x) ::close(x)
#endif
}

SSL_CTX* SslSocket::sslContext = nullptr;

SslSocket::SslSocket(const Socket socket) :
    AbstractSocket(socket),
    m_ssl(SSL_new(sslContext))
{
    if(!AbstractSocket::isValid(m_descriptor))
        return;

    SSL_set_fd(m_ssl, int(m_descriptor));

    if(SSL_accept(m_ssl) < 0)
    {
        ERR_print_errors_fp(stdout);
        CLOSE(m_descriptor);
        SSL_free(m_ssl);

        m_ssl = nullptr;
    }
}

SslSocket::~SslSocket()
{
    this->SslSocket::close();
}

void SslSocket::read(std::string &buffer)
{
    if(!m_ssl)
        return;

    int ret = 0;
    static thread_local std::shared_ptr<char[]> recvBuf(new char[SOCKET_BUF_SIZE]());

    buffer.clear();
    buffer.reserve(SOCKET_BUF_SIZE);
    do
    {
        if((ret = SSL_read(m_ssl, recvBuf.get(), SOCKET_BUF_SIZE - 1)) <= 0)
            break;  // EOF

        buffer.append(recvBuf.get(), size_t(ret));
    }
    while(ret == SOCKET_BUF_SIZE && recvBuf[SOCKET_BUF_SIZE - 1] != '\n');
}

int SslSocket::write(const char *buf, int size)
{
    if(!m_ssl || !buf)
        return 0;

    return SSL_write(m_ssl, buf, size);
}

void SslSocket::close()
{
    if(!m_ssl)
        return;

    SSL_shutdown(m_ssl);
    SSL_free(m_ssl);

    CLOSE(m_descriptor);

    m_ssl = nullptr;
}

#ifdef __linux__
ssize_t SslSocket::sendFile(int fd, off_t offset, size_t count)
{
    char *ptr = static_cast<char *>(
                mmap(nullptr, count, PROT_READ, MAP_SHARED, fd, offset));
    const int ret = this->write(ptr, count);
    munmap(ptr, count);

    return ret;
}
#endif

bool SslSocket::initializatSsl(const std::string& certFile,
                               const std::string& privateKey)
{
    if(sslContext)
        return true;

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    SSL_CTX *ctx = nullptr;

    if(!(ctx = SSL_CTX_new(SSLv23_server_method())))
        return false;

    if(SSL_CTX_use_certificate_file(ctx, certFile.c_str(),
                                    SSL_FILETYPE_PEM) <= 0 ||
       SSL_CTX_use_PrivateKey_file(ctx, privateKey.c_str(),
                                        SSL_FILETYPE_PEM) <= 0 ||
       !SSL_CTX_check_private_key(ctx))
    {
        ERR_print_errors_fp(stdout);
        SSL_CTX_free(ctx);
        return false;
    }

    sslContext = ctx;

    return true;
}

std::string SslSocket::sslVersion()
{
    return OpenSSL_version(OPENSSL_VERSION);
}

void SslSocket::cleanUpSsl()
{
    if(!sslContext)
        return;

    SSL_CTX_free(sslContext);
    sslContext = nullptr;
}
