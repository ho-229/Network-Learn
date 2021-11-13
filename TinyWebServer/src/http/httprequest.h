/**
 * @author Ho 229
 * @date 2021/7/26
 */

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include "../util/util.h"

#include <map>
#include <vector>
#include <string>
#include <unordered_set>

extern "C"
{
#ifndef _WIN32
#include <strings.h>
#endif
}

typedef std::pair<std::string,  // Name
                  std::string>  // Value
    UrlArg;

static std::unordered_set<std::string> MethodSet
    {
        "GET",
        "POST",
        "HEAD",
        "DELETE"
    };

class HttpRequest
{
public:
    HttpRequest();
    HttpRequest(const std::string& data);

    void parse(const std::string& data);

    void reset();

    std::string method() const { return m_method; }

    std::string uri() const { return m_uri; }

    const std::vector<UrlArg>& urlArguments() const { return m_urlArgs; }

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

    static auto& methodSet() { return MethodSet; }

private:
    void parseArguments(const std::string &args);

    bool parseRequestLine(std::string::size_type &offset,
                          const std::string &data);

    bool m_isValid = false;

    std::string m_method;
    std::string m_uri;
    std::string m_httpVersion;
    std::string m_body;

    std::vector<UrlArg> m_urlArgs;

    struct NocaseCompare
    {
        inline bool operator()(const std::string &left,
                               const std::string &right) const
        {
#ifdef _WIN32
            return !_stricmp(left.c_str(), right.c_str());
#else
            return strcasecmp(left.c_str(), right.c_str()) < 0;
#endif
        }
    };

    std::map<std::string,   // Name
             std::string,   // Value
             NocaseCompare> // Compare
        m_headers;
};

#endif // HTTPREQUEST_H
