#include "webserver.h"

#define LISTENQ 1024
#define BUF_SIZE 512

#include "httpservices.h"

extern "C"
{
#ifdef _WIN32
# include <WinSock2.h>
# include <WS2tcpip.h>

#define CLOSE(x) closesock(x)

#else   // Unix
# include <netdb.h>
# include <unistd.h>
# include <string.h>
# include <sys/socket.h>

#define CLOSE(x) close(x)
typedef int SOCKET;

#endif
}

#include <iostream>

WebServer::WebServer()
{

}

WebServer::~WebServer()
{
#ifdef _WIN32
    WSACleanup();
#endif
}

#ifdef _WIN32
bool WebServer::initialize()
{
    WSAData info;
    if(WSAStartup(MAKEWORD(2, 2), &info))
        return false;

    if(LOBYTE(info.wVersion) != 2 || HIBYTE(info.wVersion) != 2)
        return false;

    return true;
}
#endif

int WebServer::exec()
{
    int listenfd, connfd;
    char hostName[BUF_SIZE], port[BUF_SIZE], recvBuf[BUF_SIZE];
    socklen_t clientLen;
    sockaddr_storage clientAddr;

    listenfd = startListen(this->port);

    if(listenfd < 0)
        return -1;      // Start listen failed

    while(runnable)
    {
        clientLen = sizeof(clientAddr);
        connfd = int(accept(SOCKET(listenfd),
                            reinterpret_cast<sockaddr *>(&clientAddr), &clientLen));

        getnameinfo(reinterpret_cast<sockaddr *>(&clientAddr), clientLen,
                    hostName, BUF_SIZE, port, BUF_SIZE, 0);

        std::cout << "Accepted connection from ("<<hostName<<", "<<port<<")\n";

        std::string request, response;
        //while( > 0)
            recv(SOCKET(connfd), recvBuf, BUF_SIZE, 0);
            request.append(recvBuf, BUF_SIZE);

        std::cout << "Requset Header: \n"<<request<<std::endl;

        if(services)
            response = services->service(request);

        std::cout << "Response: \n"<<response<<std::endl;

        send(SOCKET(connfd), response.c_str(), int(response.size()), 0);
        CLOSE(SOCKET(connfd));
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
