/**
 * @author Ho 229
 * @date 2021/8/6
 */

#define LISTENQ 1024
#define BUF_SIZE 64
#define SOCKET_BUF_SIZE 4096

#include "tcpsocket.h"

#include <memory>

extern "C"
{
#ifdef _WIN32
# include <WinSock2.h>
# include <WS2tcpip.h>

#define CLOSE(x) closesocket(x)

#else   // Unix
# include <netdb.h>
# include <unistd.h>
# include <string.h>
# include <sys/socket.h>

#define CLOSE(x) close(x)
#endif
}

TcpSocket::TcpSocket(Socket descriptor) : AbstractSocket(descriptor)
{

}

TcpSocket::~TcpSocket()
{
    this->TcpSocket::close();
}

void TcpSocket::read(std::string &buffer)
{
    int ret = 0;
    std::shared_ptr<char[]> recvBuf(new char[SOCKET_BUF_SIZE]);

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
        ret = recv(m_descriptor, recvBuf.get(), SOCKET_BUF_SIZE, 0);
        buffer.append(recvBuf.get(), size_t(ret));
    }
    while(ret == SOCKET_BUF_SIZE && recvBuf[SOCKET_BUF_SIZE - 1] != '\n');
#endif
}

int TcpSocket::write(const char *buf, size_t size)
{
    return send(m_descriptor, buf, int(size), 0);
}

void TcpSocket::close()
{
    CLOSE(m_descriptor);
}

bool TcpSocket::listen(const std::string &port)
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
    getaddrinfo(nullptr, port.c_str(), &hints, &addrList);

    // Walk the list for one that we can bind to
    for(it = addrList; it; it = it->ai_next)
    {
        // Create a socket descriptor
#ifdef _WIN32
        if((m_descriptor = socket(it->ai_family, it->ai_socktype, it->ai_protocol))
            == INVALID_SOCKET)
            continue;           // Socket failed, try the next
#else
        if((m_descriptor = socket(it->ai_family, it->ai_socktype, it->ai_protocol)) < 0)
            continue;           // Socket failed, try the next
#endif

        // Eliminates "Address already in use" error from bind
        setsockopt(m_descriptor, SOL_SOCKET, SO_REUSEADDR,
                   reinterpret_cast<const char *>(&optval), sizeof(int));

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

    m_hostName = "localhost";
    m_port = port;

    m_isListening = true;
    return true;
}

TcpSocket *TcpSocket::waitForAccept()
{
    if(!m_isListening)
        return nullptr;

    sockaddr_storage clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    char hostName[BUF_SIZE], port[BUF_SIZE];

    const Socket socket = accept(
        m_descriptor, reinterpret_cast<sockaddr *>(&clientAddr), &clientLen);

    getnameinfo(reinterpret_cast<sockaddr *>(&clientAddr), clientLen,
                hostName, BUF_SIZE, port, BUF_SIZE, 0);

    TcpSocket *newConnect = new TcpSocket(socket);
    newConnect->m_hostName = hostName;
    newConnect->m_port = port;

    return newConnect;
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
