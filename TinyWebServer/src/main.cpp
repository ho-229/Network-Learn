/**
 * @author Ho 229
 * @date 2021/7/25
 */

#include <memory>
#include <iostream>

#include "until.h"
#include "webserver.h"
#include "httpservices.h"

int main(int argc, char** argv)
{
    std::cout << "Welcome to Tiny Web Server.\n";

    if(argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return -1;
    }

    auto server = std::make_shared<WebServer>();
    auto services = std::make_shared<HttpServices>();

#ifdef _WIN32
    if(server->initialize())
        std::cout << "WinSock2 load success.\n";
    else
    {
        std::cout << "WinSock2 load failed.\n";
        return -1;
    }
#endif

    // Adder Service
    server->services()->addService("GET", "/adder",
                                   [](HttpRequest* req, HttpResponse* resp) {
        int sum = 0;
        for(const auto& arg : req->urlArguments())
            sum += atoi(arg.second.c_str());

        resp->setRowHeader("Date", Until::currentDateString());

        resp->setText("<html><title>Tiny Web Server</title><body bgcolor\"#fffff\">"
                          "<span>Tiny Web Server / Adder</span><p>Result: "
                      + std::to_string(sum) + "</p></body></html>");
    });

    server->port = argv[1];
    std::cout << "Listening port: " << argv[1] << ".\n\n";

    return server->exec();
}
