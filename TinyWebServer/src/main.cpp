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
#ifdef _WIN32
    else if(signum == SIGBREAK)
#else
    else if(signum == SIGUSR2)
#endif
        server->requestQuit();
}

int main(int argc, char** argv)
{
    std::cout << "Welcome to Tiny Web Server.[" << Util::currentDateString()
              << "]\nOpenSSL version: " << SslSocket::sslVersion() << "\n";

    // -help, --help or other
    if(argc <= 2 || std::string_view("-help") == argv[1])
    {
        std::cerr << "Usage: " << argv[0] <<
            "[http-port] [https-port] [shared-directory] "
            "[certificate-file] [privateKey-file]\n\n"
            "example: ./TinyWebServer 80 443 ./shared_files";
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
//            case ExceptionEvent::ListenerError:
//                std::cerr << exception->message();
//                break;
//            case ExceptionEvent::UnknownError:
//                std::cerr << "Unknown error.\n";
//                break;
//            }
//        }
//    });

    // Add "Adder" Service
    services->onGet("/adder", [](HttpRequest *req, HttpResponse *resp) {
        int sum = 0;
        for(const auto& arg : req->urlArguments())
            sum += atoi(arg.c_str());

        resp->setRawHeader("Content-Type", "text/html; charset=utf-8");

        resp->setBody("<html><title>Tiny Web Server</title><body bgcolor\"#fffff\">"
                          "<h1>Tiny Web Server / Adder</h1><p>Result: "
                      + std::to_string(sum) + "</p></body></html>\n");
    });

    services->onGet("/hello", [](HttpRequest *, HttpResponse *resp) {
        resp->setRawHeader<true>("Content-Type", "text/html; charset=utf-8");
        *resp << "Hello world.\n";
    });

    services->onPost("/post", [](HttpRequest *req, HttpResponse *resp) {
        std::cout << "post data: " << req->body() << "\n";
        resp->setBody(std::string(req->body()));
    });

    constexpr auto build404Response = [](HttpResponse *resp) {
        resp->setRawHeader("Content-Type", "text/html; charset=utf-8");
        resp->setBody("<h2>Tiny Web Server</h2><h1>"
                      "404 Not Found"
                      "<br>∑(っ°Д°;)っ<h1>\n");
        resp->setHttpState({404, "Not Found"});
    };

    std::unique_ptr<SharedFilePool> pool;
    if(argc > 3 && fs::is_directory(argv[3]))
    {
        pool.reset(new SharedFilePool(argv[3]));

        services->onHead([&pool](HttpRequest *req, HttpResponse *resp) {
            if(auto ret = pool->get(req->uri()); ret.has_value())
            {
                resp->setRawHeader<true>("Content-Length",
                                         std::to_string(ret->fileSize));
                if(const auto type = HttpResponse::matchContentType(ret->extension);
                    !type.empty())
                    resp->setRawHeader<true>("Content-Type", type);
            }
            else
                resp->setHttpState({404, "Not Found"});
        });

        services->onGet([&pool, &build404Response]
                        (HttpRequest *req, HttpResponse *resp) {
            std::string &&uri = req->uri();

            if(uri.back() == '/')
                uri.append("index.html");

            if(auto ret = pool->get(uri); ret.has_value())
            {
                const auto file = ret.value();

                if(const auto type = HttpResponse::matchContentType(file.extension);
                    !type.empty())
                    resp->setRawHeader<true>("Content-Type", type);

                size_t offset = 0, count = file.fileSize;

                resp->setRawHeader<true>("Accept-Ranges", "bytes");
                if(const auto value = req->rawHeader("Range"); !value.empty())
                {
                    // Cache control
                    resp->setRawHeader<true>("Last-Modified", file.lastModified);
                    if(req->rawHeader("If-Modified-Since") == file.lastModified)
                    {
                        resp->setRawHeader<true>("Cache-Control", "max-age=0, public");
                        resp->setHttpState({304, "Not Modified"});
                        return;
                    }
                    else
                        resp->setRawHeader<true>("Cache-Control", "public");
                }
                else
                {
                    // Range
                    const auto range = HttpRequest::parseRange(value);
                    resp->setRawHeader<true>("Content-Range",
                                             HttpResponse::replyRange(
                                                 range, file.fileSize, offset, count));

                    resp->setHttpState({206, "Partial Content"});
                }

                resp->setBody(HttpResponse::FileBody{
                                  file.file, off_t(offset), count});
            }
            else
                build404Response(resp);
        });

        std::cout << "Shared directory: " << pool->root() << ".\n";
    }
    else
    {
        services->onGet([&build404Response](HttpRequest *, HttpResponse *resp) {
            build404Response(resp); });
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
