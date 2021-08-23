/**
 * @author Ho 229
 * @date 2021/7/26
 */

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

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

    std::string method() const { return m_method; }

    std::string uri() const { return m_uri; }

    const std::vector<UrlArg>& urlArguments() const { return m_urlArgs; }

    std::string httpVersion() const { return m_httpVersion; }

    std::string body() const { return m_body; }

    std::string rawHeader(const std::string& name) const { return m_headers.at(name); }

    std::pair<int64_t, int64_t> range() const;

    bool isEmpty() const { return m_method.empty() ||
               m_uri.empty() || m_headers.empty(); }

    bool isValid() const { return m_isValid; }

    static auto& methodSet() { return MethodSet; }

private:
    void parseArguments(const std::string& args);

    void parseRequestLine(const std::string& data);

    bool m_isValid = false;

    std::string m_method;
    std::string m_uri;
    std::string m_httpVersion;
    std::string m_body;

    std::vector<UrlArg> m_urlArgs;

    std::unordered_map<std::string, std::string> m_headers;
};

#endif // HTTPREQUEST_H
