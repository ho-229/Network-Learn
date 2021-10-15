/**
 * @author Ho 229
 * @date 2021/7/26
 */

#include "httprequest.h"
#include "httpresponse.h"
#include "httpservices.h"
#include "abstract/abstractsocket.h"

HttpServices::HttpServices()
{

}

HttpServices::~HttpServices()
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

void HttpServices::setDefaultService(const std::string &method,
                                     const Handler &handler)
{
    m_defaults[method] = std::make_shared<Handler>(handler);
}

bool HttpServices::service(AbstractSocket *const socket) const
{
    std::string raw;

    socket->read(raw);

    if(raw.empty())
        return false;

    auto request = std::make_shared<HttpRequest>(raw);
    if(!request->isValid())
        return false;

    socket->addTimes();

    auto response = std::make_shared<HttpResponse>();

    this->callHandler(request.get(), response.get());

    std::shared_ptr<char[]> sendBuf(new char[SOCKET_BUF_SIZE]);
    if(!request->isKeepAlive() || socket->times() > m_maxTimes)
        response->setRawHeader("Connection", "close");
    else
        response->setRawHeader("Connection", "keep-alive");

    response->toRawData(raw);

    if(socket->write(raw) <= 0)
        return false;

    if(response->bodyType() == HttpResponse::Stream)
    {
        if(!sendStream(socket, &response->stream()))
            return false;
    }

    return request->isKeepAlive() && socket->times() <= m_maxTimes;
}

void HttpServices::callHandler(HttpRequest *const request,
                               HttpResponse *const response) const
{
    // Find Uri -> (Method -> Handler)
    const auto handlerList = m_uris.find(request->uri());
    if(handlerList != m_uris.end())
    {
        // Find Method -> Handler
        const auto handler = handlerList->second.find(request->method());
        if(handler != handlerList->second.end())
            (*handler->second.get())(request, response);
        else
            response->buildErrorResponse(405, "Method Not Allowed");

        return;
    }

    const auto defaultHandler = m_defaults.find(request->method());
    if(defaultHandler != m_defaults.end())
    {
        (*defaultHandler->second.get())(request, response);
        return;
    }

    response->buildErrorResponse(404, "Not Found");
}

bool HttpServices::sendStream(AbstractSocket *const socket,
                              std::istream *const stream)
{
    if(!socket || stream->bad())
        return false;

    std::shared_ptr<char[]> sendBuf(new char[SOCKET_BUF_SIZE]());

    while(!stream->eof())
    {
        stream->read(sendBuf.get(), SOCKET_BUF_SIZE);

        if(socket->write(sendBuf.get(), int(stream->gcount())) <= 0)
            return false;
    }

    return true;
}
