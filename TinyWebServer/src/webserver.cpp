/**
 * @author Ho 229
 * @date 2021/7/26
 */

#include "webserver.h"

#define SOCKET_BUF_SIZE 4096

#include "event.h"
#include "tcpsocket.h"
#include "httpservices.h"

#include <thread>
#include <future>
#include <fstream>

#include <signal.h>

WebServer::WebServer() :
    m_listenSocket(new TcpSocket()),
    m_services(new HttpServices())
{
#ifdef _WIN32
    m_isLoaded = TcpSocket::initializatWsa();
#else   // Unix
    signal(SIGPIPE, SIG_IGN);       // Ignore SIGPIPE
#endif
}

WebServer::~WebServer()
{
#ifdef _WIN32
    TcpSocket::cleanUpWsa();
#endif
    m_runnable = false;

    delete m_services;
    delete m_listenSocket;
}

int WebServer::exec()
{
    if(!m_isLoaded)
    {
        ExceptionEvent event(ExceptionEvent::SocketLoadFailed);
        m_handler(&event);
        return -1;
    }

    if(!m_listenSocket->listen(m_port))
    {
        ExceptionEvent event(ExceptionEvent::ListenFailed);
        m_handler(&event);
        return -1;      // Start listen failed
    }

    m_runnable = true;
    while(m_runnable)
    {
        TcpSocket* connect = m_listenSocket->waitForAccept();

        AcceptEvent event(connect->hostName(), connect->port());
        m_handler(&event);

        auto future = std::async(&WebServer::session, this, connect);
    }

    return 0;
}

void WebServer::session(AbstractSocket *connect)
{
    std::string raw, response;

    connect->read(raw);

    if(raw.empty())
    {
        delete connect;
        return;
    }

    auto httpRequest = std::make_shared<HttpRequest>(raw);
    auto httpResponse = std::make_shared<HttpResponse>();

    m_services->service(httpRequest.get(), httpResponse.get());

    httpResponse->toRawData(response);
    connect->AbstractSocket::write(response);

    if(httpResponse->bodyType() == HttpResponse::File
        && httpRequest->method() == "GET")
    {
        std::shared_ptr<char[]> sendBuf(new char[SOCKET_BUF_SIZE]());
        std::ifstream out(httpResponse->filePath(), std::ios::binary);

        if(out)
        {
            const auto range = httpRequest->range();

            if(range.second > 0 && range.first != range.second)
            {
                out.seekg(range.first);
                while(out.tellg() < range.second)
                {
                    out.read(sendBuf.get(),
                             out.tellg() + int64_t(SOCKET_BUF_SIZE) > range.second
                                 ? range.second - out.tellg() : SOCKET_BUF_SIZE);
                    if(connect->write(sendBuf.get(), size_t(out.gcount())) <= 0)
                        break;
                }
            }
            else
            {
                while(!out.eof())
                {
                    out.read(sendBuf.get(), SOCKET_BUF_SIZE);
                    if(connect->write(sendBuf.get(), size_t(out.gcount())) <= 0)
                        break;
                }
            }
        }
    }

    delete connect;
}
