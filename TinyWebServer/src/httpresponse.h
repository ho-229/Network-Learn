/**
 * @author Ho 229
 * @date 2021/7/26
 */

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

    std::string text;

    std::map<std::string,   // Name
             std::string>   // Value
        headers;

private:
    static std::string stateString(int state);
};

#endif // HTTPRESPONSE_H
