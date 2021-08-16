# Tiny Web Server

This is a tiny `HTTP/HTTPS` web server.

## Features

| Support Headers  | Default Value |
| ---------------- | ------------- |
| Accept-Ranges | none |
| Connection | keep-alive |
| Transfer-Encoding | chunked |
| Server | Tiny Web Server |
| Date | \<auto> |
| \<Custom> | \<Custom> |

## Usage

* Command line
    ```
    Usage: ./TinyWebServer [http-port] [https-port] [shard-directory] [certificate-file] [privateKey-file]
    ```

    Example:

    ```shell
    sudo ./TinyWebServer 80 443 ./shard_files
    ```

* Browser
    ```
    http(s)://localhost/<uri>
    ```

    Example:

    ```
    http://localhost/adder?1&1
    ```

* Class `WebServer` usage  
    Example:

    ```cpp
    #include <iostream>

    #include "until.h"
    #include "webserver.h"
    #include "httpservices.h"

    int main()
    {
        auto server = std::make_shared<WebServer>();
        // Adder Service
        server->services()->addService("GET", "/adder",
                                    [](HttpRequest* req, HttpResponse* resp) {
            int sum = 0;
            for(const auto& arg : req->urlArguments())
                sum += atoi(arg.second.c_str());

            resp->setRawHeader("Date", Until::currentDateString());
            resp->setRawHeader("Content-type", "text/html; charset=utf-8");

            resp->setText("<html><title>Tiny Web Server</title><body bgcolor\"#fffff\">"
                            "<h1>Tiny Web Server / Adder</h1><p>Result: "
                        + std::to_string(sum) + "</p></body></html>\n");
        });

        server->listen("localhost", 80);    // HTTP
        return server->exec();
    }
    ```

    For more complete example, see [main.cpp](./src/main.cpp).
