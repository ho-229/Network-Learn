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

bool HttpResponse::sendStream(std::shared_ptr<std::istream> &&stream,
                              std::istream::off_type offset, size_t count)
{
    if(stream->bad())
        return false;

    m_stream = stream;

    if(count)
        m_headers["Content-Length"] = std::to_string(count);
    else    // Compute size
    {
        m_stream->seekg(0, std::ios::end);
        const auto size = m_stream->tellg() - offset;

        if(size > 0)
            m_headers["Content-Length"] = std::to_string(size);
    }
    m_stream->seekg(offset, std::ios::beg);

    m_count = count;
    m_type = BodyType::Stream;

    return true;
}

#ifdef __linux__
void HttpResponse::sendFile(int fd, off_t offset, size_t count)
{
    m_file = {fd, offset};
    m_count = count;

    m_headers["Content-Length"] = std::to_string(count);

    m_type = BodyType::File;
}
#endif

void HttpResponse::reset()
{
    m_text.clear();
    m_stream.reset();
    m_headers.clear();

    m_type = BodyType::PlainText;
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
    response.append("HTTP/1.1 ", 9).append(std::to_string(m_httpState.first))
        .append(" ", 1).append(m_httpState.second).append("\r\n", 2);

    // Headers
    response.append("Date: ", 6).append(Util::currentDateString()).append("\r\n", 2);
    response.append("Server: TinyWebServer\r\n", 23);

    for(const auto &[key, value] : m_headers)
        response.append(key).append(": ", 2).append(value).append("\r\n", 2);

    // Body
    if(m_type == BodyType::PlainText)
        response.append("\r\n", 2).append(m_text);
    else
        response.append("\r\n", 2);
}
