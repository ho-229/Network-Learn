/**
 * @author Ho 229
 * @date 2021/7/26
 */

#include "httpservices.h"
#include "abstract/abstractsocket.h"

#include <iostream>

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
    static thread_local std::string raw;

    socket->read(raw);

    if(raw.empty())
        return false;

    static thread_local auto request = std::make_shared<HttpRequest>();

    request->parse(raw);

    if(!request->isValid())
        return false;

    if(m_maxTimes)
        socket->addTimes();

    static thread_local auto response = std::make_shared<HttpResponse>();

    this->callHandler(request.get(), response.get());

    std::shared_ptr<char[]> sendBuf(new char[SOCKET_BUF_SIZE]);
    if(request->isKeepAlive() && socket->times() <= m_maxTimes)
        response->setRawHeader("Connection", "Keep-Alive");
    else
        response->setRawHeader("Connection", "Close");

    response->toRawData(raw);

    if(socket->write(raw) <= 0)
        return false;

    if(response->bodyType() == HttpResponse::Stream)
    {
        if(!sendStream(socket, &response->stream()))
            return false;
    }

    const bool ret = request->isKeepAlive() && socket->times() <= m_maxTimes;

    response->reset();

    return ret;
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
            methodIt->second.defaultHandler(request, response);  // Default handler
        else
            handlerIt->second(request, response);

        return;
    }

    // Method not found
    response->setHttpState({405, "Method Not Allowed"});
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
