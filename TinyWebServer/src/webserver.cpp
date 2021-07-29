/**
 * @author Ho 229
 * @date 2021/7/26
 */

#include "webserver.h"

#define LISTENQ 1024
#define BUF_SIZE 1024

#include "event.h"
#include "httpservices.h"

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
typedef int SOCKET;

#endif
}

#include <thread>
#include <future>

#include <iostream>

WebServer::WebServer()
    : m_services(new HttpServices())
{
#ifdef _WIN32
    WSAData info;

    m_isLoaded = !(WSAStartup(MAKEWORD(2, 2), &info) ||
                   LOBYTE(info.wVersion) != 2 || HIBYTE(info.wVersion) != 2);
#endif
}

WebServer::~WebServer()
{
#ifdef _WIN32
    WSACleanup();
#endif

    runnable = false;
    delete m_services;
}

int WebServer::exec()
{
    if(!m_isLoaded)
    {
        ExceptionEvent event(ExceptionEvent::SocketLoadFailed);
        m_handler(&event);
        return -1;
    }

    int listenfd, connfd;
    char hostName[BUF_SIZE], port[BUF_SIZE];
    socklen_t clientLen;
    sockaddr_storage clientAddr;

    if((listenfd = startListen(m_port)) < 0)
    {
        ExceptionEvent event(ExceptionEvent::ListenFailed);
        m_handler(&event);
        return -1;      // Start listen failed
    }

    while(runnable)
    {
        clientLen = sizeof(clientAddr);
        connfd = int(accept(SOCKET(listenfd),
                            reinterpret_cast<sockaddr *>(&clientAddr), &clientLen));

        getnameinfo(reinterpret_cast<sockaddr *>(&clientAddr), clientLen,
                    hostName, BUF_SIZE, port, BUF_SIZE, 0);

        AcceptEvent event(hostName, port);
        m_handler(&event);

        auto future = std::async([this](int connfd) {
            std::string raw, response;

            recvAll(connfd, raw);

            if(raw.empty())
            {
                CLOSE(SOCKET(connfd));
                return;
            }

            auto httpRequest = std::make_shared<HttpRequest>(raw);
            auto httpResponse = std::make_shared<HttpResponse>();

            m_services->service(httpRequest.get(), httpResponse.get());

            httpResponse->toRawData(response);
            send(SOCKET(connfd), response.c_str(), int(response.size()), 0);

            CLOSE(SOCKET(connfd));
        }, connfd);
    }

    return 0;
}

int WebServer::startListen(const std::string &port)
{
    addrinfo hints, *addrList, *it;
    int listenfd = -1, optval = 1;

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
        if((listenfd = int(socket(it->ai_family, it->ai_socktype, it->ai_protocol))) < 0)
            continue;       // Socket failed, try the next

        // Eliminates "Address already in use" error from bind
        setsockopt(SOCKET(listenfd), SOL_SOCKET, SO_REUSEADDR,
                   reinterpret_cast<const char *>(&optval), sizeof(int));

        // Bind the descriptor to the address
        if(!bind(SOCKET(listenfd), it->ai_addr, socklen_t(it->ai_addrlen)))
            break;          // Success

        CLOSE(SOCKET(listenfd));  // Bind failed, try the next
    }

    // Clean up
    freeaddrinfo(addrList);
    if(!it)      // No address worked
        return -1;

    // Make it a listening socket ready to accpet connection requests
    if(listen(SOCKET(listenfd), LISTENQ) < 0)
    {
        CLOSE(SOCKET(listenfd));
        return -1;
    }

    return listenfd;
}

void WebServer::recvAll(int fd, std::string &buffer)
{
    int ret = 0;
    std::shared_ptr<char[]> recvBuf(new char[BUF_SIZE]);

    buffer.clear();
#ifdef _WIN32
    do
    {
        if((ret = recv(SOCKET(fd), recvBuf.get(), BUF_SIZE, 0)) == WSAEMSGSIZE)
        {
            buffer.append(recvBuf.get(), BUF_SIZE);
            continue;
        }
        else if(ret > 0)
            buffer.append(recvBuf.get(), size_t(ret));
    }
    while(false);
#else
    while((ret = recv(SOCKET(fd), recvBuf.get(), BUF_SIZE, 0)) > 0)
        buffer.append(recvBuf.get(), size_t(ret));
#endif
}
