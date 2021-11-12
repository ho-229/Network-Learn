﻿/**
 * @author Ho 229
 * @date 2021/7/26
 */

#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <map>
#include <memory>
#include <string>
#include <istream>
#include <unordered_map>

typedef std::pair<int, std::string> HttpState;

class HttpResponse
{
public:
    enum BodyType
    {
        None,
        PlainText,
        Stream
    };

    static std::unordered_map<std::string, std::string> PermissibleStaticTypes;

    HttpResponse();

    void setText(const std::string& text);
    std::string text() const { return m_text; }

    bool setStream(std::shared_ptr<std::istream> &&stream);
    std::istream& stream() const { return *m_stream; }

    void reset();

    bool isEmpty() const { return m_text.empty() && m_stream->bad(); }

    BodyType bodyType() const { return m_type; }

    void setHttpState(const HttpState& state);
    HttpState httpState() const { return m_httpState; }

    void setRawHeader(const std::string& name, const std::string& value)
    { m_headers[name] = value; }
    std::string rawHeader(const std::string& name) const
    {
        const auto it = m_headers.find(name);
        return it == m_headers.end() ? std::string() : it->second;
    }

    void toRawData(std::string& response);

    void buildErrorResponse(int state, const std::string& message);

    inline HttpResponse& operator<<(const std::string& text)
    {
        m_text.append(text);
        m_headers["Content-Length"] = std::to_string(m_text.size());

        m_type = PlainText;

        return *this;
    }

private:
    std::pair<int, std::string> m_httpState = {200, "OK"};

    inline void initializatHeaders();

    std::string m_text;
    std::shared_ptr<std::istream> m_stream;

    std::map<std::string,   // Name
             std::string>   // Value
        m_headers;

    BodyType m_type = None;
};

#endif // HTTPRESPONSE_H