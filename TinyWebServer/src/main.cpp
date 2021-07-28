/**
 * @author Ho 229
 * @date 2021/7/25
 */

#include <memory>
#include <iostream>

#include "event.h"
#include "until.h"
#include "webserver.h"
#include "httpservices.h"

void test(Event *)
{
    std::cout << "On event\n";
}

int main(int argc, char** argv)
{
    std::cout << "Welcome to Tiny Web Server.[" << Until::currentDateString() << "]\n";

    if(argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return -1;
    }

    auto server = std::make_shared<WebServer>();

    server->installEventHandler([](Event *event) {
        if(event->type() == Event::AcceptEvent)
        {
            AcceptEvent *accept = static_cast<AcceptEvent *>(event);
            std::cout << "Accepted connection from ("
                      << accept->hostName() <<", "<< accept->port() <<")\n";
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
                std::cerr << "Listen port failed.\n";
                break;
            case ExceptionEvent::UnknownError:
                std::cerr << "Unknown error.\n";
                break;
            }
        }
    });

    // Adder Service
    server->services()->addService("GET", "/adder",
                                   [](HttpRequest* req, HttpResponse* resp) {
        int sum = 0;
        for(const auto& arg : req->urlArguments())
            sum += atoi(arg.second.c_str());

        resp->setRowHeader("Date", Until::currentDateString());
        resp->setRowHeader("Content-type", "text/html; charset=ascii");

        resp->setText("<html><title>Tiny Web Server</title><body bgcolor\"#fffff\">"
                          "<span>Tiny Web Server / Adder</span><p>Result: "
                      + std::to_string(sum) + "</p></body></html>");
    });

    server->setPort(argv[1]);
    std::cout << "Listening port: " << argv[1] << ".\n\n";

    return server->exec();
}
