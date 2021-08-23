/**
 * @author Ho 229
 * @date 2021/7/25
 */

#include <iostream>

#include "until.h"
#include "webserver.h"
#include "sslsocket.h"
#include "httpservices.h"

#ifdef _WIN32
# if _MSC_VER >= 1600
#  pragma execution_character_set("utf-8")
# endif
#endif

int main(int argc, char** argv)
{
    std::cout << "Welcome to Tiny Web Server.[" << Until::currentDateString()
              << "]\nOpenSSL version: " << SslSocket::sslVersion() << "\n";

    // -help
    if(argc >= 2 && std::string(argv[1]) == "-help")
    {
        std::cerr << "Usage: " << argv[0] << "[http-port] [https-port] [shard-directory] "
                                             "[certificate-file] [privateKey-file]\n";
        return 1;
    }

    auto server = std::make_shared<WebServer>();

    if(argc == 6)
    {
        if(SslSocket::initializatSsl(argv[4], argv[5]))
            std::cerr << "OpenSSL initializat successful.\n";
        else
            std::cerr << "OpenSSL initializat failed.\n";
    }

    server->installEventHandler([](Event *event) {
        if(event->type() == Event::ConnectEvent)
        {
            ConnectEvent *accept = static_cast<ConnectEvent *>(event);

            auto socket = accept->socket();

            if(socket->sslEnable())
                std::cout << "[SSL] ";
            else
                std::cout << "[TCP] ";

            if(accept->state() == ConnectEvent::Accpet)
                std::cout << "Accepted connection from ";
            else
                std::cout << "Connection closed ";

            std::cout << socket->hostName() <<":"<< socket->port() <<"\n";
        }
        else if(event->type() == Event::ExceptionEvent)
        {
            ExceptionEvent *exception = static_cast<ExceptionEvent *>(event);

            switch (exception->error())
            {
            case ExceptionEvent::SocketLoadFailed:
                std::cerr << "WinSock2 load failed.\n";
                break;
            case ExceptionEvent::ListenFailed:
                std::cerr << exception->message();
                break;
            case ExceptionEvent::UnknownError:
                std::cerr << "Unknown error.\n";
                break;
            }
        }
    });

    // Add "Adder" Service
    server->services()->addService("GET", "/adder",
                                   [](HttpRequest* req, HttpResponse* resp) {
        int sum = 0;
        for(const auto& arg : req->urlArguments())
            sum += atoi(arg.second.c_str());

        resp->setRawHeader("Content-type", "text/html; charset=utf-8");

        resp->setText("<html><title>Tiny Web Server</title><body bgcolor\"#fffff\">"
                          "<h1>Tiny Web Server / Adder</h1><p>Result: "
                      + std::to_string(sum) + "</p></body></html>\n");
    });

    if(argc > 3)
    {
        server->services()->setWorkDir(argv[3]);
        std::cout << "Shard directory: " << argv[3] << ".\n";
    }

    if(argc >= 2)   // HTTP
    {
        std::cout << "Listening HTTP port: " << argv[1] << "\n";
        server->listen(ANY_HOST, argv[1], false);
    }

    if(argc >= 3 && SslSocket::isSslAvailable())    // HTTPS
    {
        std::cout << "Listening HTTPS port: " << argv[2] << "\n";
        server->listen(ANY_HOST, argv[2], true);
    }

    std::cout << "\n";

    return server->exec();
}
