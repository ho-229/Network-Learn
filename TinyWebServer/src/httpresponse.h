/**
 * @author Ho 229
 * @date 2021/7/26
 */

#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <map>
#include <string>

typedef std::pair<int, std::string> HttpState;

class HttpResponse
{
public:
    HttpResponse();

    void setText(const std::string text) { m_body = text; }
    std::string text() const { return m_body; }

    void write(const void* data, size_t len)
    { m_body.append(reinterpret_cast<const char *>(data), len); }

    void reset();

    bool isEmpty() const { return m_body.empty() || m_headers.empty(); }

    void setHttpState(const HttpState& state);
    HttpState httpState() const { return m_httpState; }

    void setRowHeader(const std::string& name, const std::string& value)
    { m_headers[name] = value; }
    std::string rawHeader(const std::string& name) const { return m_headers.at(name); }

    void toRowData(std::string& response);

private:
    std::pair<int, std::string> m_httpState = {200, "OK"};

    std::string m_body;

    std::map<std::string,   // Name
             std::string>   // Value
        m_headers;
};

#endif // HTTPRESPONSE_H
