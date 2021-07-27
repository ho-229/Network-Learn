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

public:
    std::string method;
    std::string uri;
    std::map<std::string, std::string> headers;     // Unused various
    std::vector<UrlArg> urlArgs;

private:
    void buildArgs(const std::string& args);
};

#endif // HTTPREQUEST_H
