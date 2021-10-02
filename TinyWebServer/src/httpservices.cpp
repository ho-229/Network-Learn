/**
 * @author Ho 229
 * @date 2021/7/26
 */

#include "httpservices.h"

HttpServices::HttpServices() : m_workDir(".")
{

}

void HttpServices::addService(const std::string &method,
                              const std::string &uri, const Handler &handler)
{
    auto it = m_uris.find(uri);
    if(it == m_uris.end())
    {
        MethodHandler methodHandler;
        methodHandler[method] = std::make_shared<Handler>(handler);
        m_uris[uri] = methodHandler;
    }
    else
        it->second[method] = std::make_shared<Handler>(handler);
}

void HttpServices::setWorkDir(const std::filesystem::path &path)
{
    if(fs::is_directory(path))
        m_workDir = path;
}

void HttpServices::service(HttpRequest* httpRequest, HttpResponse* httpResponse) const
{
    // Find Uri -> (Method -> Handler)
    const auto handlerList = m_uris.find(httpRequest->uri());
    if(handlerList != m_uris.end())
    {
        // Find Method -> Handler
        const auto handler = handlerList->second.find(httpRequest->method());
        if(handler != handlerList->second.end())
            (*handler->second.get())(httpRequest, httpResponse);
        else
            httpResponse->buildErrorResponse(405, "Method Not Allowed");
    }
    else
    {
        fs::path filePath(m_workDir.string() + httpRequest->uri());
        if(fs::is_regular_file(filePath))
            httpResponse->setFilePath(filePath);
        else
            httpResponse->buildErrorResponse(404, "Not Found");
    }
}
