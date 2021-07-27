#include "httpresponse.h"

HttpResponse::HttpResponse()
{
    headers["Server"] = "Tiny Web Server";
    headers["Content-type"] = "text/html";
    headers["Connection"] = "closed";
}

std::string HttpResponse::toString()
{
    std::string result;

    headers["Content-lenght"] = std::to_string(text.size());

    result.append(stateString(state));
    for(const auto &[key, value] : headers)
        result.append(key + ": " + value + "\r\n");
    result.append("\r\n" + text);

    return result;
}

std::string HttpResponse::stateString(int state)
{
    switch (state)
    {
    case 200:
        return "HTTP/1.1 200 OK\r\n";
    case 403:
        return "HTTP/1.1 403 Forbidden\r\n";
    case 404:
        return "HTTP/1.1 404 Not found\r\n";
    case 501:
        return "HTTP/1.1 501 Notimplemented\r\n";
    }

    return {};
}
