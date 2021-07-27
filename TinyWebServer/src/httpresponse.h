#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <map>
#include <string>

class HttpResponse
{
public:
    HttpResponse();

    std::string toString();

public:
    int state = 0;

    std::string userData;

    std::map<std::string,   // Name
             std::string>   // Value
        headers;

private:
    static std::string stateString(int state);
};

#endif // HTTPRESPONSE_H
