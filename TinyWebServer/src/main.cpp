/**
 * @author Ho 229
 * @date 2021/7/25
 */

#define PROFILER_ENABLE 0

#include <fstream>
#include <iostream>
#include <filesystem>

#include <signal.h>

#if PROFILER_ENABLE
# include <gperftools/profiler.h>
#endif

namespace fs = std::filesystem;

#include "webserver.h"
#include "httpservices.h"

#include "until/until.h"
#include "core/sslsocket.h"

#ifdef _WIN32
# if _MSC_VER >= 1600
#  pragma execution_character_set("utf-8")
# endif
#endif

static auto server = std::make_shared<WebServer>();

void signalHandler(int signum)
{
    if(signum == SIGINT)
        server->quit();
}

int main(int argc, char** argv)
{
    std::cout << "Welcome to Tiny Web Server.[" << Until::currentDateString()
              << "]\nOpenSSL version: " << SslSocket::sslVersion() << "\n";

    // -help, --help or other
    if(argc >= 2 && argv[1][0] == '-')
    {
        std::cerr << "Usage: " << argv[0] << "[http-port] [https-port] [shared-directory] "
                                             "[certificate-file] [privateKey-file]\n";
        return 1;
    }

    if(argc == 6)
    {
        if(SslSocket::initializatSsl(argv[4], argv[5]))
            std::cerr << "OpenSSL initializat successful.\n";
        else
            std::cerr << "OpenSSL initializat failed.\n";
    }

    server->setServices(new HttpServices);

//    server->installEventHandler([](Event *event) {
//        if(event->type() == Event::ConnectEvent)
//        {
//            ConnectEvent *accept = static_cast<ConnectEvent *>(event);

//            auto socket = accept->socket();

//            if(socket->sslEnable())
//                std::cerr << "[SSL] ";
//            else
//                std::cerr << "[TCP] ";

//            if(accept->state() == ConnectEvent::Accpet)
//                std::cerr << "Accepted connection descriptor ";
//            else
//                std::cerr << "Connection closed descriptor ";

//            std::cerr << socket->descriptor() <<"\n";
//        }
//        else if(event->type() == Event::ExceptionEvent)
//        {
//            ExceptionEvent *exception = static_cast<ExceptionEvent *>(event);

//            switch (exception->error())
//            {
//            case ExceptionEvent::SocketLoadFailed:
//                std::cerr << "WinSock2 load failed.\n";
//                break;
//            case ExceptionEvent::ListenFailed:
//                std::cerr << exception->message();
//                break;
//            case ExceptionEvent::UnknownError:
//                std::cerr << "Unknown error.\n";
//                break;
//            }
//        }
//    });

    // Add "Adder" Service
    server->services()->addService("GET", "/adder",
                                   [](HttpRequest *req, HttpResponse *resp) {
        int sum = 0;
        for(const auto& arg : req->urlArguments())
            sum += atoi(arg.second.c_str());

        resp->setRawHeader("Content-type", "text/html; charset=utf-8");

        resp->setText("<html><title>Tiny Web Server</title><body bgcolor\"#fffff\">"
                          "<h1>Tiny Web Server / Adder</h1><p>Result: "
                      + std::to_string(sum) + "</p></body></html>\n");
    });

    server->services()->addService("GET", "/hello",
                                   [](HttpRequest *, HttpResponse *resp) {
        resp->setRawHeader("Content-type", "text/html; charset=utf-8");
        *resp << "Hello world.\n";
    });

    if(argc > 3 && fs::is_directory(argv[3]))
    {
        const std::string workPath(argv[3]);

        server->services()->setDefaultService("GET", [workPath](HttpRequest *req,
                                                                HttpResponse *resp) {
            fs::path path(workPath + req->uri());

            auto out = new std::ifstream(path, std::ios::binary);

            if(out->good())
                resp->setStream(out);
            else
                resp->setHttpState({404, "Not Found"});
        });

        std::cout << "Shared directory: " << workPath << ".\n";
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

    signal(SIGINT, signalHandler);

#if PROFILER_ENABLE
    ProfilerStart("test_capture.prof");
#endif

    const int ret = server->exec();

#if PROFILER_ENABLE
    ProfilerStop();
#endif

    return ret;
}
