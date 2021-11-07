# Tiny Web Server

This is a tiny `HTTP/HTTPS` web server.

The idea of doing this project originated from the TinyWebServer of CS:APP, I hope to learn network programming from this project.

* Webbench

> CPU: Intel i5-7500 3.40GHz x 4  
> RAM: 8GB  
> OS: Manjaro KDE

* Short connection
![image](./screenshot/close.png)

* Keep alive
![image](./screenshot/keep-alive.png)

## Features

| Support Headers  | Default Value |
| ---------------- | ------------- |
| Connection | keep-alive |
| Content-Length | \<auto> |
| Server | TinyWebServer |
| Date | \<auto> |
| \<Custom> | \<Custom> |

## Usage

* Command line

    ```shell
    Usage: ./TinyWebServer [http-port] [https-port] [shared-directory] [certificate-file] [privateKey-file]
    ```

    Example:

    ```shell
    sudo ./TinyWebServer 80 443 ./shared_files
    ```

* Browser

    ```shell
    http(s)://localhost/<uri>
    ```

    Example:

    ```shell
    http://localhost/adder?1&1
    ```

* Class `WebServer` usage  
    Example:

    ```cpp
    #include "webserver.h"
    #include "httpservices.h"

    int main()
    {
        auto server = std::make_shared<WebServer>();
        auto services = new HttpServices();

        server->setServices(services);

        // Adder Service
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

        server->listen("localhost", 80);    // HTTP
        return server->exec();
    }
    ```

    For more complete example, see [main.cpp](./src/main.cpp).
