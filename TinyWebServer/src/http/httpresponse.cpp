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

bool HttpResponse::sendStream(std::unique_ptr<std::istream> &&stream, size_t count)
{
    if(stream->bad())
        return false;

    m_stream = std::move(stream);

    const auto start = m_stream->tellg();

    if(count)
    {
        m_stream->seekg(count, std::ios::cur);

        if(m_stream->fail())
            goto seekFailed;

        m_count = count;
        m_headers["Content-Length"] = std::to_string(count);
    }
    else
    {
    seekFailed:
        m_stream->seekg(0, std::ios::end);

        m_count = 0;        // Send stream til EOF
        if(const auto size = m_stream->tellg())
            m_headers["Content-Length"] = std::to_string(size - start);
    }

    m_stream->seekg(start);

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
    m_httpState = {200, "OK"};
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
