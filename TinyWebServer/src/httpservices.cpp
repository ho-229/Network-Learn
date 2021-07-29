/**
 * @author Ho 229
 * @date 2021/7/26
 */

#include "httpservices.h"

HttpServices::HttpServices()
{

}

void HttpServices::addService(const std::string &method,
                              const std::string &uri, Handler handler)
{
    auto it = m_uriHandlers.find(uri);
    if(it == m_uriHandlers.end())
    {
        MethodHandler methodHandler;
        methodHandler[method] = std::make_shared<Handler>(handler);
        m_uriHandlers[uri] = methodHandler;
    }
    else
        it->second[method] = std::make_shared<Handler>(handler);
}

void HttpServices::service(HttpRequest* httpRequest, HttpResponse* httpResponse)
{
    // Find Uri -> (Method -> Handler)
    if(m_uriHandlers.find(httpRequest->uri()) != m_uriHandlers.end())
    {
        // Find Method -> Handler
        const auto& methodHandler = m_uriHandlers[httpRequest->uri()];
        if(methodHandler.find(httpRequest->method()) != methodHandler.end())
        {
            const auto& handler = *methodHandler.at(httpRequest->method()).get();
            handler(httpRequest, httpResponse);
        }
        else
            httpResponse->buildErrorResponse(405, "Method Not Allowed");
    }
    else
        httpResponse->buildErrorResponse(404, "Not Found");
}
