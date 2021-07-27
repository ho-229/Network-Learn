/**
 * @author Ho 229
 * @date 2021/7/25
 */

#include <memory>
#include <iostream>

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

    std::cout << "Listening port: " << argv[1] << ".\n\n";

    // Adder Service
    services->addService("GET", "/adder", [](HttpRequest* req, HttpResponse* resp) {
        int sum = 0;
        for(const auto& arg : req->urlArgs)
            sum += atoi(arg.second.c_str());

        resp->text += "<html><title>Tiny Web Server</title><body bgcolor\"#fffff\">"
                          "<span>Tiny Web Server / Adder</span><p>Result: "
                          + std::to_string(sum) + "</p></body></html>";

        resp->state = 200;
    });

    server->port = argv[1];
    server->services = services.get();

    return server->exec();
}
