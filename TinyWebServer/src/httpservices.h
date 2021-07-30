/**
 * @author Ho 229
 * @date 2021/7/26
 */

#ifndef HTTPSERVICES_H
#define HTTPSERVICES_H

#include <map>
#include <string>
#include <memory>
#include <functional>

#include "httprequest.h"
#include "httpresponse.h"

typedef std::function<void(HttpRequest *, HttpResponse *)> Handler;
typedef std::map<std::string, std::shared_ptr<Handler>> MethodHandler;

class HttpServices
{
public:
    explicit HttpServices();

    void addService(const std::string &method, const std::string& uri,
                    const Handler& handler);

    void service(HttpRequest* httpRequest, HttpResponse* httpResponse);

private:
    std::map<std::string,       // URI
             MethodHandler>     // Method -> Handler
        m_uriHandlers;
};

#endif // HTTPSERVICES_H
