/**
 * @author Ho 229
 * @date 2021/7/26
 */

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include "../util/util.h"
#include "../util/headermap.h"

#include <vector>
#include <unordered_set>

extern "C"
{
#ifndef _WIN32
#include <strings.h>
#endif
}

typedef std::pair<std::string,  // Name
                  std::string>  // Value
    UrlArgument;

class HttpRequest
{
public:
    HttpRequest();
    HttpRequest(const std::string& data);

    void parse(const std::string& data);

    void reset();

    std::string method() const { return m_method; }

    std::string uri() const { return m_uri; }

    const std::vector<UrlArgument>& urlArguments() const { return m_urlArgs; }

    std::string httpVersion() const { return m_httpVersion; }

    std::string body() const { return m_body; }

    std::string rawHeader(const std::string &name) const
    {
        const auto it = m_headers.find(name);
        return it == m_headers.end() ? std::string() : it->second;
    }

    std::pair<int64_t, int64_t> range() const;

    bool isKeepAlive() const
    {
        const auto it = m_headers.find("Connection");
        return it == m_headers.end() ? true :
#ifdef _WIN32
                   _stricmp(it->second.c_str(), "close");
#else
                   strcasecmp(it->second.c_str(), "close");
#endif
    }

    bool isEmpty() const { return m_method.empty() ||
               m_uri.empty() || m_headers.empty(); }

    bool isValid() const { return m_isValid; }

    static std::unordered_set<std::string> MethodSet;

private:
    void parseArguments(const std::string &args);

    bool parseRequestLine(std::string::size_type &offset,
                          const std::string &data);

    bool m_isValid = false;

    std::string m_method;
    std::string m_uri;
    std::string m_httpVersion;
    std::string m_body;

    std::vector<UrlArgument> m_urlArgs;

    HeaderMap m_headers;
};

#endif // HTTPREQUEST_H
