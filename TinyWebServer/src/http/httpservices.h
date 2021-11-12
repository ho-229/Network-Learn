/**
 * @author Ho 229
 * @date 2021/7/26
 */

#ifndef HTTPSERVICES_H
#define HTTPSERVICES_H

#include <memory>
#include <istream>
#include <unordered_map>

#include "httprequest.h"
#include "httpresponse.h"
#include "../abstract/abstractservices.h"

typedef std::function<void(HttpRequest *, HttpResponse *)> Handler;

class HttpServices : public AbstractServices
{
public:
    explicit HttpServices();
    ~HttpServices() override;

    void addService(const std::string &method, const std::string& uri,
                    const Handler& handler);

    void setDefaultService(const std::string& method,
                           const Handler& handler);

    bool process(AbstractSocket *const socket) const override;

private:
    struct UriHandler
    {
        std::unordered_map<std::string, // URI
                           Handler>     // Handler
            uriHandlers;

        Handler defaultHandler;
    };

    void callHandler(HttpRequest *const request,
                     HttpResponse *const response) const;

    static bool sendStream(AbstractSocket *const socket,
                           std::istream * const stream);

    std::unordered_map<std::string,     // Method -> {URI -> Handler}
                       UriHandler>      // URI -> Handler
        m_services;
};

#endif // HTTPSERVICES_H
