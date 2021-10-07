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
#include "abstract/abstractservices.h"

typedef std::unordered_map<std::string,                 // Method
                           std::shared_ptr<Handler>>    // Handler
    MethodHandler;

class HttpServices : public AbstractServices
{
public:
    explicit HttpServices();
    ~HttpServices() override;

    void addService(const std::string &method, const std::string& uri,
                    const Handler& handler) override;

    void setDefaultService(const std::string& method,
                           const Handler& handler) override;

    bool service(AbstractSocket *const socket) const override;

private:
    void callHandler(HttpRequest *const request,
                     HttpResponse *const response) const;

    static bool sendStream(AbstractSocket *const socket,
                           std::istream * const stream);

    std::unordered_map<std::string,       // URI
                       MethodHandler>     // Method -> Handler
        m_uris;

    std::unordered_map<std::string,                 // Method
                       std::shared_ptr<Handler>>    // Handler
        m_defaults;
};

#endif // HTTPSERVICES_H
