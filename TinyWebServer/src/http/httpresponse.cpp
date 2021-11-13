/**
 * @author Ho 229
 * @date 2021/7/26
 */

#include "httpresponse.h"

#include "../util/util.h"

#ifdef _WIN32
# if _MSC_VER >= 1600
#  pragma execution_character_set("utf-8")
# endif
#endif

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
    this->initializatHeaders();
}

void HttpResponse::setText(const std::string& text)
{
    m_text = text;
    m_headers["Content-Length"] = std::to_string(m_text.size());

    m_type = PlainText;
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

    m_type = Stream;

    return true;
}

void HttpResponse::reset()
{
    m_text.clear();
    m_stream.reset();
    m_type = None;

    m_headers.clear();
    this->initializatHeaders();
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
            .append(m_httpState.second).append("\r\n");

    // Headers
    for(const auto &[key, value] : m_headers)
        response.append(key).append(": ").append(value).append("\r\n");

    if(m_type == PlainText)
        response.append("\r\n").append(m_text);
    else
        response.append("\r\n");
}

void HttpResponse::buildErrorResponse(int state, const std::string &message)
{
    this->setRawHeader("Content-Type", "text/html; charset=utf-8");

    this->setHttpState({404, "Not found"});
    this->setText("<h2>Tiny Web Server</h2><h1>" + std::to_string(state)
                  + " " + message + "<br>∑(っ°Д°;)っ<h1>\n");
}

inline void HttpResponse::initializatHeaders()
{
    m_headers["Server"] = "TinyWebServer";
    m_headers["Date"] = Util::currentDateString();
}
