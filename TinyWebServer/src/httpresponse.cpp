/**
 * @author Ho 229
 * @date 2021/7/26
 */

#include "httpresponse.h"

HttpResponse::HttpResponse()
{
    m_headers["Server"] = "Tiny Web Server";
    m_headers["Content-type"] = "text/html";
    m_headers["Connection"] = "close";
}

void HttpResponse::reset()
{
    m_body.clear();

    m_headers.clear();
    m_headers["Server"] = "Tiny Web Server";
    m_headers["Content-type"] = "text/html";
    m_headers["Connection"] = "close";

    m_httpState = {200, "OK"};
}

void HttpResponse::setHttpState(const HttpState &state)
{
    if(state.second.empty())
        return;

    m_httpState = state;
}

void HttpResponse::toRowData(std::string &response)
{
    m_headers["Content-length"] = std::to_string(m_body.size());

    response.append("HTTP/1.1 " + std::to_string(m_httpState.first)
                    + m_httpState.second + "\r\n");

    for(const auto &[key, value] : m_headers)
        response.append(key + ": " + value + "\r\n");

    response.append("\r\n" + m_body);
}

