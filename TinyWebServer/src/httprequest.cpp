/**
 * @author Ho 229
 * @date 2021/7/26
 */

#include "httprequest.h"

#include <sstream>
#include <regex>

HttpRequest::HttpRequest()
{

}

HttpRequest::HttpRequest(const std::string &data)
{
    this->parse(data);
}

void HttpRequest::parse(const std::string &data)
{
    std::stringstream stream(data);
    std::string line;

    const std::regex express("(.+)\\s*:\\s*(.+)");
    std::smatch result;

    bool isFirstLine = true;
    while(std::getline(stream, line) && line != "\r\n")
    {
        if(isFirstLine)
        {
            this->parseRequestLine(line);
            isFirstLine = false;
        }
        else
            if(std::regex_search(line, result, express))
                m_headers[result[1]] = result[2];
    }

    while(!stream.eof())
        stream >> m_body;
}

std::pair<int64_t, int64_t> HttpRequest::range() const
{
    const auto it = m_headers.find("Range");

    if(it == m_headers.end())
        return {0, 0};

    const std::regex express("bytes=(\\d+)-(\\d+)");
    std::smatch result;

    if(!std::regex_match(it->second, result, express))
        return {0, 0};

    return {atoll(result[1].str().c_str()), atoll(result[2].str().c_str())};
}

void HttpRequest::parseRequestLine(const std::string &data)
{
    std::regex express("(\\w+)\\s(.+)\\sHTTP\\W(\\S+)");
    std::smatch result;

    if(std::regex_search(data, result, express))
    {
        int i = 0;
        for(const auto& it : result)
        {
            if(i == 1)
                m_method = it.str();
            else if(i == 2)
            {
                const std::string str = it.str();
                std::regex express("(.+)\\?(.+)");
                std::smatch result;
                if(std::regex_match(str, result, express))
                {
                    m_uri = result[1];
                    this->parseArguments(result[2]);
                }
                else
                    m_uri = str;
            }
            else if(i == 3)
                m_httpVersion = it.str();
            ++i;
        }
    }
}

void HttpRequest::parseArguments(const std::string &args)
{
    std::stringstream stream(args);
    std::string line;

    const std::regex express("(.*)=(.*)");
    std::smatch result;

    while(std::getline(stream, line, '&'))
    {
        if(std::regex_search(line, result, express))
            m_urlArgs.push_back({result[1], result[2]});
        else
            m_urlArgs.push_back({{}, line});
    }
}
