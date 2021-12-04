/**
 * @author Ho 229
 * @date 2021/7/26
 */

#include "httpservices.h"
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
    static thread_local std::string raw;

    socket->read(raw);

    if(raw.empty())
        return false;

    static thread_local auto request = std::make_unique<HttpRequest>();

    request->parse(raw);

    if(!request->isValid())
        return false;

    if(m_maxTimes)
        socket->addTimes();

    static thread_local auto response = std::make_unique<HttpResponse>();

    this->callHandler(request.get(), response.get());

    std::shared_ptr<char[]> sendBuf(new char[SOCKET_BUF_SIZE]);
    if(request->isKeepAlive() && socket->times() <= m_maxTimes)
        response->setRawHeader("Connection", "Keep-Alive");
    else
        response->setRawHeader("Connection", "Close");

    response->toRawData(raw);

    Util::DestroyFunction destroy([] {
        request->reset();
        response->reset();
    });

    if(response->bodyType() == HttpResponse::BodyType::PlainText)
    {
        if(socket->write(raw) <= 0)
            return false;
    }
    else
    {
//#ifdef __linux__
//        socket->setOption(IPPROTO_TCP, TCP_CORK, 1);
//#endif
        if(socket->write(raw) <= 0)
            return false;
//#ifdef __linux__
//        socket->setOption(IPPROTO_TCP, TCP_CORK, 0);
//#endif
        if(response->bodyType() == HttpResponse::BodyType::Stream)
        {
            if(!socket->sendStream(response->m_stream.get(), response->m_count))
                return false;
        }
#ifdef __linux__
        else    // File
        {
            if(socket->sendFile(response->m_file.fd,
                                response->m_file.offset,
                                response->m_count) < 0)
                return false;
        }
#endif
    }

    const bool ret = request->isKeepAlive() && socket->times() <= m_maxTimes;

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
            methodIt->second.defaultHandler(request, response);     // Default handler
        else
            handlerIt->second(request, response);

        return;
    }

    // Method not found
    response->setHttpState({405, "Method Not Allowed"});
}
