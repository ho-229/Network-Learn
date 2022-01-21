/**
 * @author Ho 229
 * @date 2021/8/7
 */

#include "sslsocket.h"

#include <openssl/err.h>

extern "C"
{
#ifdef _WIN32
#else   // Unix
# include <netdb.h>
# include <unistd.h>
# include <string.h>
# include <sys/mman.h>
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
        AbstractSocket::close(m_descriptor);
        SSL_free(m_ssl);

        m_ssl = nullptr;
    }
}

SslSocket::~SslSocket()
{
    if(this->SslSocket::isValid())
        this->SslSocket::close();
}

ssize_t SslSocket::read(char *buf, size_t count)
{
    if(m_isListener || !m_ssl || !buf || !count)
        return -1;

    ssize_t ret = 0;
    size_t leftSize = count;

    do
    {
        if(ret = SSL_read(m_ssl, buf, count > INT32_MAX ? INT32_MAX : int(count));
                ret <= 0)
            return ret;

        leftSize -= size_t(ret);
        buf += ret;
    }
    while(leftSize);

    return ssize_t(count);
}

ssize_t SslSocket::write(const char *buf, size_t count)
{
    if(m_isListener || !m_ssl || !buf || !count)
        return -1;

    ssize_t ret = 0;
    size_t leftSize = count;

    do
    {
        if(ret = SSL_write(m_ssl, buf, count > INT32_MAX ? INT32_MAX : int(count));
                ret <= 0)
            return ret;

        leftSize -= size_t(ret);
        buf += ret;
    }
    while(leftSize);

    return ssize_t(count);
}

void SslSocket::close()
{
    if(!m_ssl)
        return;

    SSL_shutdown(m_ssl);
    SSL_free(m_ssl);

    AbstractSocket::close(m_descriptor);

    m_ssl = nullptr;
}

ssize_t SslSocket::sendFile(File file, off_t offset, size_t count)
{
#ifdef _WIN32
    // Create a page space
    HANDLE page = CreateFileMapping(file, nullptr, PAGE_READONLY,
                                    static_cast<DWORD>(uint64_t(count) >> 32),
                                    static_cast<DWORD>(count & 0xffffffff),
                                    nullptr);
    if(!page)
        return -1;

    // Map data from the file
    void *ptr = MapViewOfFile(page, FILE_MAP_READ,
                              static_cast<DWORD>(uint64_t(offset) >> 32),
                              static_cast<DWORD>(
                                  static_cast<unsigned long>(offset) & 0xffffffff),
                              count);
    if(!ptr)
        return -1;

    const ssize_t ret = this->write(reinterpret_cast<const char *>(ptr), count);

    UnmapViewOfFile(ptr);
    CloseHandle(page);

    return ret;
#else
    char *ptr = static_cast<char *>(
                mmap(nullptr, count, PROT_READ, MAP_SHARED, file, offset));
    const int ret = this->write(ptr, count);
    munmap(ptr, count);

    return ret;
#endif
}

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
