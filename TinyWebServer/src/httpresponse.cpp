/**
 * @author Ho 229
 * @date 2021/7/26
 */

#include "httpresponse.h"
#include "until.h"

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
}

void HttpResponse::setHttpState(const HttpState &state)
{
    if(state.second.empty())
        return;

    m_httpState = state;
}

void HttpResponse::toRawData(std::string &response)
{
    m_headers["Content-length"] = std::to_string(m_body.size());

    response.append("HTTP/1.1 " + std::to_string(m_httpState.first)
                    + m_httpState.second + "\r\n");

    for(const auto &[key, value] : m_headers)
        response.append(key + ": " + value + "\r\n");

    response.append("\r\n" + m_body);
}

void HttpResponse::buildErrorResponse(int state, const std::string &message)
{
    this->setRawHeader("Content-type", "text/html; charset=utf-8");
    this->setRawHeader("Date", Until::currentDateString());

    this->setHttpState({404, "Not found"});
    m_body = Until::errorHtml(state, message);
}

