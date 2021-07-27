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

std::string HttpServices::service(const std::string request)
{
    auto httpRequest = std::make_shared<HttpRequest>(request);
    auto httpResponse = std::make_shared<HttpResponse>();

    if(m_uriHandlers.find(httpRequest->uri) != m_uriHandlers.end())
    {
        auto methodHandler = m_uriHandlers[httpRequest->uri];
        if(methodHandler.find(httpRequest->method) != methodHandler.end())
        {
            auto handler = *methodHandler[httpRequest->method].get();
            handler(httpRequest.get(), httpResponse.get());
        }
    }

    return httpResponse->toString();
}
