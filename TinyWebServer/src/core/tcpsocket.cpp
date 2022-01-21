/**
 * @author Ho 229
 * @date 2021/8/6
 */

#include "tcpsocket.h"

extern "C"
{
#ifdef _WIN32
#else   // Unix
# include <sys/sendfile.h>
#endif
}

TcpSocket::TcpSocket(const Socket socket) : AbstractSocket(socket)
{

}

TcpSocket::~TcpSocket()
{
    if(this->TcpSocket::isValid())
        this->TcpSocket::close();
}

ssize_t TcpSocket::read(char *buf, size_t count)
{
    if(m_isListener || !buf || !count)
        return -1;

    ssize_t ret = 0;
    size_t leftSize = count;

    do
    {
#ifdef _WIN32
        if(ret = recv(m_descriptor, buf, leftSize > INT32_MAX ? INT32_MAX :
                      int(count), 0); ret <= 0)
            return ssize_t(count - leftSize);
#else   // Unix
        if(ret = ::read(m_descriptor, buf, leftSize); ret <= 0)
        {
            if(errno == EINTR)              // Interrupted by signal handler return
                continue;                   // and call read() again
            else
                return count - leftSize;    // errno set by read()
        }
#endif

        leftSize -= size_t(ret);
        buf += ret;
    }
    while(leftSize);

    return ssize_t(count);
}

ssize_t TcpSocket::write(const char *buf, size_t count)
{
    if(m_isListener || !buf || !count)
        return -1;

    size_t leftSize = count;
    ssize_t ret;

    do
    {
#ifdef _WIN32
        if(ret = send(m_descriptor, buf, leftSize > INT32_MAX ? INT32_MAX :
                      int(count), 0); ret <= 0)
            return ssize_t(count - leftSize);

#else   // Unix
        if(ret = ::write(m_descriptor, buf, count); ret <= 0)
        {
            if(errno == EINTR)              // Interrupted by signal handler return
                continue;                   // and call write() again
            else
                return count - leftSize;    // errno set by write()
        }
#endif

        leftSize -= size_t(ret);
        buf += ret;
    }
    while(leftSize);

    return ssize_t(count);
}

void TcpSocket::close()
{
    AbstractSocket::close(m_descriptor);
}

ssize_t TcpSocket::sendFile(File file, off_t offset, size_t count)
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
#else   // Unix
    return sendfile64(m_descriptor, file, &offset, count);
#endif
}
