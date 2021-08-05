/**
 * @author Ho 229
 * @date 2021/7/26
 */

#include "httpresponse.h"

#include "until.h"

#ifdef _WIN32
# if _MSC_VER >= 1600
#  pragma execution_character_set("utf-8")
# endif
#endif

HttpResponse::HttpResponse()
{
    m_headers["Server"] = "Tiny Web Server";
    m_headers["Connection"] = "close";
    m_headers["Accept-Ranges"] = "bytes";
}

void HttpResponse::setFilePath(const fs::path &path)
{
    const fs::directory_entry entry(path);
    if(entry.is_regular_file())
    {
        m_filePath = path;
        m_type = File;
    }
}

void HttpResponse::reset()
{
    m_text.clear();
    m_filePath.clear();

    m_headers.clear();
    m_headers["Server"] = "Tiny Web Server";
    m_headers["Connection"] = "close";
    m_headers["Accept-Ranges"] = "bytes";
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

    m_headers["Content-Length"] = std::to_string(
        m_type == Normal ? m_text.size() : fs::directory_entry(m_filePath).file_size() - 1);

    // Response line
    response.append("HTTP/1.1 " + std::to_string(m_httpState.first) + ' '
                    + m_httpState.second + "\r\n");

    // Headers
    for(const auto &[key, value] : m_headers)
        response.append(key + ": " + value + "\r\n");

    if(m_type == Normal)
        response.append("\r\n" + m_text);
    else
        response.append("\r\n");
}

void HttpResponse::buildErrorResponse(int state, const std::string &message)
{
    this->setRawHeader("Content-Type", "text/html; charset=utf-8");
    this->setRawHeader("Date", Until::currentDateString());

    this->setHttpState({404, "Not found"});
    m_text = "<h2>Tiny Web Server</h2><h1>" + std::to_string(state)
             + " " + message + "<br>∑(っ°Д°;)っ<h1>\n";
}

void HttpResponse::buildFileResponse(const fs::path &filePath)
{
    const auto it = PermissibleStaticType.find(filePath.extension().string());
    if(it != PermissibleStaticType.end())
        this->setRawHeader("Content-Type", it->second);

    this->setRawHeader("Date", Until::currentDateString());
    this->setFilePath(filePath);
}
