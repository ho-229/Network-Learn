/**
 * @author Ho 229
 * @date 2021/7/25
 */

#define PROFILER_ENABLE 0
#define _GLIBCXX_SANITIZE_VECTOR 0

#include <fstream>
#include <iostream>

#include <signal.h>

#if PROFILER_ENABLE
# include <gperftools/profiler.h>
#endif

#include "webserver.h"
#include "util/util.h"
#include "core/sslsocket.h"
#include "http/httpservices.h"
#include "util/sharedfilepool.h"

#ifdef _WIN32
# if _MSC_VER >= 1600
#  pragma execution_character_set("utf-8")
# endif
#endif

static auto server = std::make_unique<WebServer>();

void signalHandler(int signum)
{
    if(signum == SIGINT)
        server->quit();
}

int main(int argc, char** argv)
{
    std::cout << "Welcome to Tiny Web Server.[" << Util::currentDateString()
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

    auto services = new HttpServices;
    server->setServices(services);

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
    services->addService("GET", "/adder",
                                   [](HttpRequest *req, HttpResponse *resp) {
        int sum = 0;
        for(const auto& arg : req->urlArguments())
            sum += atoi(arg.second.c_str());

        resp->setRawHeader("Content-type", "text/html; charset=utf-8");

        resp->setText("<html><title>Tiny Web Server</title><body bgcolor\"#fffff\">"
                          "<h1>Tiny Web Server / Adder</h1><p>Result: "
                      + std::to_string(sum) + "</p></body></html>\n");
    });

    services->addService("GET", "/hello",
                                   [](HttpRequest *, HttpResponse *resp) {
        resp->setRawHeader("Content-type", "text/html; charset=utf-8");
        *resp << "Hello world.\n";
    });

    services->addService("POST", "/post",
                         [](HttpRequest *req, HttpResponse *resp) {
                             std::cout << "post data: " << req->body() << "\n";
                             resp->setText(req->body());
                         });

#ifdef __linux__
    std::shared_ptr<SharedFilePool> pool;
    if(argc > 3 && fs::is_directory(argv[3]))
    {
        pool.reset(new SharedFilePool(argv[3]));

        services->setDefaultService("GET", [&pool](HttpRequest *req, HttpResponse *resp) {
            if(auto ret = pool->get(req->uri()); !ret.has_value())
            {
                resp->setRawHeader("Content-Type", "text/html; charset=utf-8");
                resp->setText("<h2>Tiny Web Server</h2><h1>"
                              "404 Not Found"
                              "<br>∑(っ°Д°;)っ<h1>\n");
                resp->setHttpState({404, "Not Found"});
            }
            else
                resp->sendFile(ret.value().fd, 0, ret.value().fileSize);
        });

        std::cout << "Shared directory: " << pool->root() << ".\n";
    }
#else
    if(argc > 3 && fs::is_directory(argv[3]))
    {
        const std::string workPath(argv[3]);

        services->setDefaultService("GET", [workPath](HttpRequest *req, HttpResponse *resp) {
            fs::path path(workPath + req->uri());
            if(!fs::is_regular_file(path) ||
                    !resp->sendStream(std::shared_ptr<std::istream>(
                    new std::ifstream(path, std::ios::binary))))
            {
                resp->setRawHeader("Content-Type", "text/html; charset=utf-8");
                resp->setText("<h2>Tiny Web Server</h2><h1>"
                              "404 Not Found"
                              "<br>∑(っ°Д°;)っ<h1>\n");
                resp->setHttpState({404, "Not Found"});
            }
        });

        std::cout << "Shared directory: " << workPath << ".\n";
    }
#endif
    else
    {
        services->setDefaultService("GET", [](HttpRequest *, HttpResponse *resp) {
            resp->setRawHeader("Content-Type", "text/html; charset=utf-8");
            resp->setText("<h2>Tiny Web Server</h2><h1>"
                          "404 Not Found"
                          "<br>∑(っ°Д°;)っ<h1>\n");
            resp->setHttpState({404, "Not Found"});
        });
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
