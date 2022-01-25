/**
 * @author Ho 229
 * @date 2021/7/26
 */

#include "httpservices.h"

#include "../define.h"
#include "../abstract/abstractsocket.h"

HttpServices::HttpServices()
{

}

HttpServices::~HttpServices()
{

}

void HttpServices::addService(const std::string &method,
                              const std::string &uri, const Handler &handler)
{
    auto it = m_services.find(method);
    if(it == m_services.end())
        m_services[method] = {{{uri, handler}},
                              {[](HttpRequest *, HttpResponse *resp) {
                                  resp->setHttpState({404, "Not Found"}); }}};
    else
        it->second.uriHandlers[uri] = handler;
}

void HttpServices::setDefaultService(const std::string &method,
                                     const Handler &handler)
{
    auto it = m_services.find(method);
    if(it == m_services.end())
        m_services[method] = {{}, handler};
    else
        it->second.defaultHandler = handler;
}

bool HttpServices::process(AbstractSocket *const socket) const
{
    static thread_local std::unique_ptr<HttpRequest> request(new HttpRequest);

    socket->read(request->m_rawData);

    request->parse();

    if(!request->isValid())
        return false;

    if(m_maxTimes)
        socket->addTimes();

    static thread_local std::string buffer;
    static thread_local std::unique_ptr<HttpResponse> response(new HttpResponse);

    const Util::ScopeFunction func([] {
        request->reset();
        response->reset();
        buffer.clear();
    });

    if(m_isAutoKeepAlive)
        response->setKeepAlive(request->isKeepAlive());

    this->callHandler(request.get(), response.get());

    response->toRawData(buffer);

#if defined(__linux__) && TCP_CORK_ENABLE
    socket->setOption(IPPROTO_TCP, TCP_CORK, 1);
#endif

    // Write Header and text body
    if(socket->write(buffer) != buffer.size())
        return false;

#if defined(__linux__) && TCP_CORK_ENABLE
    socket->setOption(IPPROTO_TCP, TCP_CORK, 0);
#endif

    const bool ok = response->visitBody(
                Util::overloaded {
                    [](const HttpResponse::StringBody &) { return true; },

                    [&socket](HttpResponse::StreamBody &stream) -> bool
                    { return socket->sendStream(stream.get()); },

                    [&socket](const HttpResponse::FileBody &file) -> bool
                    { return socket->sendFile(
                      file.file, file.offset, file.count) == file.count; }
                });;

    if(!ok)
        return false;

    return response->isKeepAlive() && socket->times() <= m_maxTimes;
}

void HttpServices::callHandler(HttpRequest *const request,
                               HttpResponse *const response) const
{
    // Find Method -> {URI -> Handler}
    const auto methodIt = m_services.find(request->method());
    if(methodIt != m_services.end())
    {
        // URI -> Handler
        const auto handlerIt = methodIt->second.uriHandlers.find(request->uri());
        if(handlerIt == methodIt->second.uriHandlers.end())         // URI not found
            methodIt->second.defaultHandler(request, response);     // Default handler
        else
            handlerIt->second(request, response);

        return;
    }

    // Method not found
    response->setHttpState({405, "Method Not Allowed"});
}
