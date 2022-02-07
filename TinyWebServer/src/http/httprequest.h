/**
 * @author Ho 229
 * @date 2021/7/26
 */

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include "../util/headermap.h"

#include <vector>
#include <ostream>

class HttpRequest
{
public:
    using Body = std::string_view;

    HttpRequest();

    void reset();

    std::string method() const
    { return m_isValid ? std::string(m_method) : std::string(); }

    std::string uri() const { return m_isValid ? m_uri : std::string(); }

    const std::vector<std::string>& urlArguments() const { return m_urlArguments; }

    std::string httpVersion() const
    { return m_isValid ? std::string(m_httpVersion) : std::string(); }

    const Body body() const { return m_body; }

    std::string rawHeader(const std::string &name) const
    {
        const auto it = m_headers.find(name);
        return it == m_headers.end() ? std::string()
                                     : std::string(it->second);
    }

    const auto& rawHeaders() const { return m_headers; }

    bool isKeepAlive() const { return m_isKeepAlive; }

    bool isValid() const { return m_isValid; }

    friend std::ostream& operator<<(std::ostream &stream, const HttpRequest &req)
    {
        stream << "Method: " << req.method()
               << "|\nURI: " << req.uri()
               << "|\nVersion: " << req.httpVersion()

               << "|\n\nUrl Arguments:\n";

        for(const auto& arg : req.urlArguments())
            stream << arg << "|\n";

        stream << "\nHeaders:\n";

        for(const auto& item : req.rawHeaders())
            stream << item.first << "| => |" << item.second << "|\n";

        stream << "\nBody:\n" << req.body();

        return stream;
    }

    static std::pair<size_t, size_t> parseRange(const std::string &range);

private:
    friend class HttpServices;

    void parse();
    bool parseRequestLine(std::string::size_type &offset,
                          const std::string &data);
    bool parseRequestHeaders(std::string::size_type &offset,
                             const std::string &data);

    bool m_isValid = false;

    std::string m_rawData;

    std::string_view m_method;
    std::string      m_uri;
    std::string_view m_httpVersion;
    Body m_body;

    std::vector<std::string> m_urlArguments;

    HeaderMap<std::string_view, std::string_view> m_headers;

    bool m_isKeepAlive = true;
};

#endif // HTTPREQUEST_H
