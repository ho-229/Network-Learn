/**
 * @author Ho 229
 * @date 2021/7/26
 */

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include "../util/util.h"
#include "../util/headermap.h"

#include <vector>
#include <ostream>
#include <unordered_set>

extern "C"
{
#ifndef _WIN32
#include <strings.h>
#endif
}

class HttpRequest
{
public:
    HttpRequest();

    void reset();

    std::string method() const
    { return m_isValid ? std::string(m_method) : std::string(); }

    std::string uri() const;

    const std::vector<std::string>& urlArguments() const { return m_urlArguments; }

    std::string httpVersion() const
    { return m_isValid ? std::string(m_httpVersion) : std::string(); }

    std::string body() const
    { return m_isValid ? std::string(m_body) : std::string(); }

    std::string rawHeader(const std::string &name) const
    {
        const auto it = m_headers.find(name);
        return it == m_headers.end() ? std::string()
                                     : std::string(it->second);
    }

    const auto& rawHeaders() const { return m_headers; }

    bool isKeepAlive() const
    {
        const auto it = m_headers.find("Connection");
        return it == m_headers.end() ? true :
#ifdef _WIN32
                   _stricmp(it->second.data(), "close");
#else
                   strcasecmp(it->second.data(), "close");
#endif
    }

    bool isEmpty() const { return m_method.empty() ||
               m_uri.empty() || m_headers.empty(); }

    bool isValid() const { return m_isValid; }

    static std::unordered_set<std::string> MethodSet;

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
            stream << item.first << "==>" << item.second << "|\n";

        stream << "\nBody:\n" << req.body();

        return stream;
    }

private:
    friend class HttpServices;

    void parse();
    bool parseRequestLine(std::string::size_type &offset,
                          const std::string &data);

    bool m_isValid = false;

    std::string m_rawData;

    std::string_view m_method;
    std::string_view m_uri;
    std::string_view m_httpVersion;
    std::string_view m_body;

    std::vector<std::string> m_urlArguments;

    HeaderMap<std::string_view, std::string_view> m_headers;
};

#endif // HTTPREQUEST_H
