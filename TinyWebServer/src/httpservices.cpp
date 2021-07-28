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

bool HttpServices::service(const std::string& request, std::string& response)
{
    auto httpRequest = std::make_shared<HttpRequest>(request);
    auto httpResponse = std::make_shared<HttpResponse>();

    // Find Uri -> (Method -> Handler)
    if(m_uriHandlers.find(httpRequest->uri()) != m_uriHandlers.end())
    {
        // Find Method -> Handler
        auto methodHandler = m_uriHandlers[httpRequest->uri()];
        if(methodHandler.find(httpRequest->method()) != methodHandler.end())
        {
            auto handler = *methodHandler[httpRequest->method()].get();
            handler(httpRequest.get(), httpResponse.get());

            if(!httpResponse->isEmpty())
            {
                httpResponse->toRowData(response);
                return true;
            }
        }
    }

    return false;
}
