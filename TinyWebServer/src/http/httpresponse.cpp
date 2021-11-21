/**
 * @author Ho 229
 * @date 2021/7/26
 */

#include "httpresponse.h"

#include "../util/util.h"

std::unordered_map<std::string, std::string> HttpResponse::PermissibleStaticTypes
    {
        {".html", "text/html"},
        {".css", "text/css"},
        {".js", "text/javascript"},
        {".png", "image/png"},
        {".jpg", "image/jpg"},
        {".jpeg", "image/jpeg"},
        {".gif", "image/gif"},
        {".svg", "image/svg+xml"},
        {".txt", "text/plain"},
        {".pdf", "application/pdf"},
        {".ico", "image/x-icon"},
        {".swf", "application/x-shockwave-flash"}
    };

HttpResponse::HttpResponse()
{

}

void HttpResponse::setText(const std::string& text)
{
    m_text = text;
    m_headers["Content-Length"] = std::to_string(m_text.size());

    m_type = BodyType::PlainText;
}

bool HttpResponse::setStream(std::shared_ptr<std::istream> &&stream)
{
    if(stream->bad())
        return false;

    m_stream = stream;

    m_stream->seekg(0, std::ios::end);

    const auto size = m_stream->tellg();
    if(size > 0)
        m_headers["Content-Length"] = std::to_string(size);

    m_stream->seekg(0, std::ios::beg);

    m_type = BodyType::Stream;

    return true;
}

void HttpResponse::reset()
{
    m_text.clear();
    m_stream.reset();
    m_type = BodyType::None;

    m_headers.clear();
}

void HttpResponse::setHttpState(const HttpState &state)
{
    if(state.second.empty())
        return;

    m_httpState = state;
}

void HttpResponse::toRawData(std::string &response)
{
    response.clear();

    // Response line
    response.append("HTTP/1.1 ").append(std::to_string(m_httpState.first))
        .append(" ").append(m_httpState.second).append("\r\n");

    // Headers
    response.append("Date: ").append(Util::currentDateString()).append("\r\n");
    response.append("Server: TinyWebServer\r\n");

    for(const auto &[key, value] : m_headers)
        response.append(key).append(": ").append(value).append("\r\n");

    // Body
    if(m_type == BodyType::PlainText)
        response.append("\r\n").append(m_text);
    else
        response.append("\r\n");
}
