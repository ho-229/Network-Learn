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
    if(argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return -1;
    }

    auto server = std::make_shared<WebServer>();
    auto services = std::make_shared<HttpServices>();

#ifdef _WIN32
    if(server->initialize())
        std::cout << "Load socket success.\n";
    else
    {
        std::cout << "Socket load failed.\n";
        return -1;
    }
#endif

    std::cout << "Tiny Web Server is running ...\n\n";

    services->addService("GET", "/adder", [](HttpRequest* req, HttpResponse* resp) {
        int sum = 0;
        for(const auto& arg : req->urlArgs)
            sum += atoi(arg.second.c_str());

        resp->userData += R"(<html><title>Tiny Web Server</title><body bgcolor"#fffff"><h>Tiny Web Server / Adder</h><p>Result: )" + std::to_string(sum) + R"(</p></body></html>)";

        resp->state = 200;
    });

    server->port = argv[1];
    server->services = services.get();

    return server->exec();
}
