/**
 * @author Ho 229
 * @date 2021/11/21
 */

#include "abstractsocket.h"

extern "C"
{
#ifdef _WIN32
#else
# include <fcntl.h>
# include <netdb.h>
# include <string.h>
# include <netinet/tcp.h>
#endif
}

AbstractSocket::AbstractSocket(const Socket socket) : m_descriptor(socket)
{
#if SOCKET_INFO_ENABLE
    if(!isValid(socket))
        return;

    // initialize information about socket
    sockaddr_in addr;
    socklen_t len = sizeof (addr);

    getpeername(m_descriptor, reinterpret_cast<sockaddr *>(&addr), &len);

    char hostName[32];
    m_hostName = inet_ntop(addr.sin_family, &addr.sin_addr,
                           hostName, sizeof (hostName));
    m_port = std::to_string(ntohs(addr.sin_port));
#endif
}

void AbstractSocket::read(std::string &buffer)
{
    ssize_t ret = 0;

    do
    {
        const auto start = buffer.size();
        buffer.resize(start + SOCKET_BUF_SIZE);     // Alloc space

        if(ret = this->read(buffer.data() + start, SOCKET_BUF_SIZE); ret <= 0)
            buffer.resize(start);
        else if(const auto newSize = start + size_t(ret); newSize != buffer.size())
            buffer.resize(newSize);                 // Fit space
    }
    while(ret == SOCKET_BUF_SIZE);
}

bool AbstractSocket::sendStream(std::istream * const stream)
{
    if(!stream)
        return false;

    static thread_local std::unique_ptr<char[]> buffer(new char[SOCKET_BUF_SIZE]());

    while(!stream->eof())
    {
        stream->read(buffer.get(), SOCKET_BUF_SIZE);
        if(this->write(buffer.get(), size_t(stream->gcount())) != stream->gcount())
            return false;
    }

    return true;
}

bool AbstractSocket::listen(const std::string &hostName, const std::string &port)
{
    if(m_isListener || hostName.empty() || port.empty())
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

        AbstractSocket::close(m_descriptor);    // Bind failed, try the next
    }

    // Clean up
    freeaddrinfo(addrList);
    if(!it)      // No address worked
        return false;

    // Make it a listening socket ready to accpet connection requests
    if(::listen(m_descriptor, LISTENQ) < 0)
    {
        AbstractSocket::close(m_descriptor);
        return false;
    }

// Set non-blocking
#ifdef OS_WINDOWS
    u_long mode = 1;
    ioctlsocket(m_descriptor, FIONBIO, &mode);
#else
    int flags;
    if((flags = fcntl(m_descriptor, F_GETFL, nullptr)) < 0 ||
        fcntl(m_descriptor, F_SETFL, flags | O_NONBLOCK) == -1)
        return false;

    //this->setOption(IPPROTO_TCP, TCP_NODELAY, 1);
#endif

#if SOCKET_INFO_ENABLE
    m_hostName = hostName;
    m_port = port;
#endif

    m_isListener = true;
    return true;
}

Socket AbstractSocket::accept() const
{
    if(!m_isListener)
        return {};

    sockaddr_storage addr;
    socklen_t len = sizeof(addr);

#if defined (OS_WINDOWS)   // Windows
    const Socket socket = ::accept(
        m_descriptor, reinterpret_cast<sockaddr *>(&addr), &len);
#elif defined (OS_LINUX)   // Linux
    const Socket socket = ::accept4(
        m_descriptor, reinterpret_cast<sockaddr *>(&addr), &len, O_NONBLOCK);
#else                      // Unix
    const Socket socket = ::accept(
        m_descriptor, reinterpret_cast<sockaddr *>(&addr), &len);

    int flags;
    if((flags = fcntl(m_descriptor, F_GETFL, nullptr)) < 0 ||
        fcntl(m_descriptor, F_SETFL, flags | O_NONBLOCK) == -1)
        return INVALID_SOCKET;
#endif

    return socket;
}

#ifdef _WIN32
bool AbstractSocket::initializatWsa()
{
    WSAData info;

    return !(WSAStartup(MAKEWORD(2, 2), &info) ||
             LOBYTE(info.wVersion) != 2 || HIBYTE(info.wVersion) != 2);
}

void AbstractSocket::cleanUpWsa()
{
    WSACleanup();
}
#endif

