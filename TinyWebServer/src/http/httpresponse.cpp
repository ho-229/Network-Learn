/**
 * @author Ho 229
 * @date 2021/7/26
 */

#include "httpresponse.h"

HttpResponse::HttpResponse()
{

}

void HttpResponse::setBody(const StringBody &text)
{
    if(text.empty())
        return;

    m_headers["Content-Length"] = std::to_string(text.size());

    new (&m_body) Body(text);
}

void HttpResponse::setBody(StreamBody &&stream)
{
    if(stream->bad())
        return;

    const auto start = stream->tellg();
    stream->seekg(0, std::ios::end);

    if(const auto size = stream->tellg(); size)
        m_headers["Content-Length"] = std::to_string(size);

    stream->seekg(start);

    new (&m_body) Body(std::move(stream));
}

void HttpResponse::setBody(const FileBody &file)
{
    if(!file.count)
        return;

    m_headers["Content-Length"] = std::to_string(file.count);

    new (&m_body) Body(file);
}

void HttpResponse::reset()
{
    this->visitBody(Util::overloaded {
                        [](StringBody &text) { text.clear(); },
                        [](StreamBody &stream) { stream.reset(); },
                        [](FileBody &file) { file = {}; }
                    });
    new (&m_body) Body();

    m_headers.clear();

    m_httpState = {200, "OK"};

    m_isKeepAlive = true;
}

bool HttpResponse::isValid() const
{
    return m_httpState.first >= 200 && m_httpState.first <= 500 &&
           !m_httpState.second.empty();
}

void HttpResponse::setKeepAlive(bool isKeepAlive)
{
    m_headers["Connection"] = isKeepAlive ? "keep-alive" : "close";
    m_isKeepAlive = isKeepAlive;
}

void HttpResponse::setHttpState(const HttpState &state)
{
    if(state.second.empty())
        return;

    m_httpState = state;
}

std::string HttpResponse::matchContentType(const std::string &extension)
{
    static const std::unordered_map<std::string_view, std::string_view> mimeTypes
    {
        {".js",   "application/javascript"},
        {".json", "application/json"},
        {".pdf",  "application/pdf"},
        {".swf",  "application/x-shockwave-flash"},
        {".xml",  "application/xml"},
        {".htm",  "text/html"},
        {".html", "text/html"},
        {".css",  "text/css"},
        {".txt",  "text/plain"},
        {".png",  "image/png"},
        {".jpg",  "image/jpg"},
        {".jpeg", "image/jpeg"},
        {".gif",  "image/gif"},
        {".svg",  "image/svg+xml"},
        {".ico",  "image/x-icon"},
    };

    const auto it = mimeTypes.find(extension);
    return it == mimeTypes.end() ? "" : std::string{it->second};
}

std::string HttpResponse::replyRange(std::pair<size_t, size_t> range,
                                     const size_t total, size_t &offset, size_t &count)
{
    // To valid range
    if(range.first >= total || range.second >= total)
    {
        range.first = 0;
        range.second = total - 1;
    }
    else if(range.second <= range.first)
        range.second = total - 1;

    offset = range.first;
    count = range.second - range.first + 1;

    return std::string("bytes ")
            .append(std::to_string(range.first)).append("-")
            .append(std::to_string(range.second)).append("/")
            .append(std::to_string(total));
}

void HttpResponse::toRawData(std::string &response)
{
    // Response line
    response.append("HTTP/1.1 ", 9).append(std::to_string(m_httpState.first))
        .append(" ", 1).append(m_httpState.second).append("\r\n", 2);

    // Headers
    response.append("Date: ", 6).append(Util::currentDateString()).append("\r\n", 2);
    response.append("Server: TinyWebServer\r\n", 23);

    for(const auto &[key, value] : m_headers)
        response.append(key).append(": ", 2).append(value).append("\r\n", 2);

    // Body
    if(m_body.type == Body::Text)
        response.append("\r\n", 2).append(m_body.text);
    else
        response.append("\r\n", 2);
}
