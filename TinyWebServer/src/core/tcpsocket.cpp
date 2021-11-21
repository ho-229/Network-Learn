﻿/**
 * @author Ho 229
 * @date 2021/8/6
 */

#define LISTENQ 1024

#include "tcpsocket.h"

extern "C"
{
#ifdef _WIN32
# define CLOSE(x) closesocket(x)
#else   // Unix
# include <fcntl.h>
# include <netdb.h>
# include <unistd.h>
# include <string.h>
# include <netinet/tcp.h>
# include <sys/sendfile.h>

# define CLOSE(x) ::close(x)
#endif
}

TcpSocket::TcpSocket(const Socket socket) : AbstractSocket(socket)
{

}

TcpSocket::~TcpSocket()
{
    this->TcpSocket::close();
}

void TcpSocket::read(std::string &buffer)
{
    if(m_isListening)
        return;

    int ret = 0;
    static thread_local std::shared_ptr<char[]> recvBuf(new char[SOCKET_BUF_SIZE]());

    buffer.clear();
    buffer.reserve(SOCKET_BUF_SIZE);
#ifdef _WIN32
    do
    {
        if((ret = recv(m_descriptor, recvBuf.get(), SOCKET_BUF_SIZE, 0)) == WSAEMSGSIZE)
        {
            buffer.append(recvBuf.get(), SOCKET_BUF_SIZE);
            continue;
        }
        else if(ret > 0)
            buffer.append(recvBuf.get(), size_t(ret));
    }
    while(false);
#else   // Unix
    do
    {
        do
            ret = ::read(m_descriptor, recvBuf.get(), SOCKET_BUF_SIZE);
        while(ret <= 0 && errno == EINTR);

        if(ret <= 0)
            break;  // EOF

        buffer.append(recvBuf.get(), size_t(ret));
    }
    while(ret == SOCKET_BUF_SIZE && recvBuf[SOCKET_BUF_SIZE - 1] != '\n');
#endif
}

int TcpSocket::write(const char *buf, int size)
{
    if(m_isListening || !buf || !size)
        return -1;

#ifdef _WIN32
    return send(m_descriptor, buf, size, 0);
#else
    size_t leftSize = size;
    ssize_t writtenSize;
    const char *bufptr = buf;

    while(leftSize > 0)
    {
        if((writtenSize = ::write(m_descriptor, bufptr, leftSize)) <= 0)
        {
            if(errno == EINTR)      // Interrupted by signal handler return
                writtenSize = 0;    // and call write() again
            else
                return -1;          // errno set by write()
        }

        leftSize -= writtenSize;
        bufptr += writtenSize;
    }

    return size;
#endif
}

void TcpSocket::close()
{
    CLOSE(m_descriptor);
}

#ifdef __linux__
ssize_t TcpSocket::sendFile(int fd, off_t offset, size_t count)
{
    return sendfile(m_descriptor, fd, &offset, count);
}
#endif

bool TcpSocket::listen(const std::string &hostName, const std::string &port, bool sslEnable)
{
    if(m_isListening)
        return false;

    addrinfo hints, *addrList, *it;
    int optval = 1;

    // Get a list of potential server addresses
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_socktype = SOCK_STREAM;                // Accept connections
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG     // ... on any IP address
                     | AI_NUMERICSERV;              // ... using port number
    getaddrinfo(hostName.c_str(), port.c_str(), &hints, &addrList);

    // Walk the list for one that we can bind to
    for(it = addrList; it; it = it->ai_next)
    {
        // Create a socket descriptor
        if(!AbstractSocket::isValid(
                (m_descriptor = socket(it->ai_family, it->ai_socktype, it->ai_protocol))))
            continue;

        // Eliminates "Address already in use" error from bind
        this->setOption(SOL_SOCKET, SO_REUSEADDR, optval);

        // Bind the descriptor to the address
        if(!bind(m_descriptor, it->ai_addr, socklen_t(it->ai_addrlen)))
            break;              // Success

        CLOSE(m_descriptor);    // Bind failed, try the next
    }

    // Clean up
    freeaddrinfo(addrList);
    if(!it)      // No address worked
        return false;

    // Make it a listening socket ready to accpet connection requests
    if(::listen(m_descriptor, LISTENQ) < 0)
    {
        CLOSE(m_descriptor);
        return false;
    }

    // Set non-blocking
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(m_descriptor, FIONBIO, &mode);
#else
    int flags;
    if((flags = fcntl(m_descriptor, F_GETFL, nullptr)) < 0 ||
            fcntl(m_descriptor, F_SETFL, flags | O_NONBLOCK) == -1)
        return false;

//    flags = 1;
//    setsockopt(m_descriptor, IPPROTO_TCP, TCP_NODELAY, &flags, sizeof (int));
#endif

    m_sslEnable = sslEnable;

#if SOCKET_INFO_ENABLE
    m_hostName = hostName;
    m_port = port;
#endif

    m_isListening = true;
    return true;
}

Socket TcpSocket::accept() const
{
    if(!m_isListening)
        return {};

    sockaddr_storage addr;
    socklen_t len = sizeof(addr);

    const Socket socket = ::accept(
        m_descriptor, reinterpret_cast<sockaddr *>(&addr), &len);

    return socket;
}

#ifdef _WIN32
bool TcpSocket::initializatWsa()
{
    WSAData info;

    return !(WSAStartup(MAKEWORD(2, 2), &info) ||
             LOBYTE(info.wVersion) != 2 || HIBYTE(info.wVersion) != 2);
}

void TcpSocket::cleanUpWsa()
{
    WSACleanup();
}
#endif
