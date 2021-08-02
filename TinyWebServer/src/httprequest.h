/**
 * @author Ho 229
 * @date 2021/7/26
 */

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <map>
#include <vector>
#include <string>

typedef std::pair<std::string,  // Name
                  std::string>  // Value
    UrlArg;

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

    std::string rawHeader(const std::string& name) const { return m_headers.at(name); }

    std::pair<int64_t, int64_t> range() const;

private:
    void buildArgs(const std::string& args);
    void parseRequestLine(const std::string& data);

    std::string m_method;
    std::string m_uri;
    std::string m_httpVersion;

    std::vector<UrlArg> m_urlArgs;

    std::map<std::string, std::string> m_headers;
};

#endif // HTTPREQUEST_H
