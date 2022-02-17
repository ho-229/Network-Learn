/**
 * @author Ho 229
 * @date 2021/7/26
 */

#ifndef HTTPSERVICES_H
#define HTTPSERVICES_H

#include "httprequest.h"
#include "httpresponse.h"
#include "../abstract/abstractservices.h"

typedef std::function<void(HttpRequest *, HttpResponse *)> Handler;

class HttpServices : public AbstractServices
{
public:
    explicit HttpServices();
    ~HttpServices() override;

    void addService(const std::string& method, const std::string& uri,
                    const Handler& handler);

    /**
     * @brief Set method default service
     */
    void setDefaultService(const std::string& method,
                           const Handler& handler);

    /**
     * @brief Set default service
     */
    void setDefaultService(const Handler& handler)
    { m_defaultHandler = handler; }

    void setAutoKeepAlive(bool isEnable) { m_isAutoKeepAlive = isEnable; }
    bool isAutoKeepAlive() const { return m_isAutoKeepAlive; }

    inline void onGet(const std::string& uri, const Handler& handler)
    { this->addService("GET", uri, handler); }

    inline void onGet(const Handler& handler)
    { this->setDefaultService("GET", handler); }

    inline void onHead(const std::string& uri, const Handler& handler)
    { this->addService("HEAD", uri, handler); }

    inline void onHead(const Handler& handler)
    { this->setDefaultService("HEAD", handler); }

    inline void onPost(const std::string& uri, const Handler& handler)
    { this->addService("POST", uri, handler); }

    inline void onPost(const Handler& handler)
    { this->setDefaultService("POST", handler); }

    inline void onPut(const std::string& uri, const Handler& handler)
    { this->addService("PUT", uri, handler); }

    inline void onPut(const Handler& handler)
    { this->setDefaultService("PUT", handler); }

    inline void onDelete(const std::string& uri, const Handler& handler)
    { this->addService("DELETE", uri, handler); }

    inline void onDelete(const Handler& handler)
    { this->setDefaultService("DELETE", handler); }

protected:
    bool process(AbstractSocket *const socket) const override;

private:
    void callHandler(HttpRequest *const request,
                     HttpResponse *const response) const;

    struct UriHandler
    {
        std::unordered_map<std::string, // URI
                           Handler>     // Handler
            uriHandlers;

        Handler defaultHandler;
    };

    Handler m_defaultHandler;

    std::unordered_map<std::string,     // Method -> {URI -> Handler}
                       UriHandler>      // URI -> Handler
        m_services;

    bool m_isAutoKeepAlive = true;
};

#endif // HTTPSERVICES_H
