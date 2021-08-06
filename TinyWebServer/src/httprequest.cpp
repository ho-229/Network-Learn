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
    std::stringstream stream(data);
    std::string line;

    for(int i = 0; std::getline(stream, line, ' '); ++i)
    {
        if(i == 0)
            m_method = line;
        else if(i == 1)
        {
            std::string::size_type pos;

            while(true)
            {
                if((pos = line.find('?')) == std::string::npos)
                    m_uri = line;
                else
                {
                    if(line[pos - 1] == '\\')
                        continue;

                    m_uri = line.substr(0, pos);
                    this->parseArguments(line.substr(pos + 1));
                }

                break;
            }
        }
        else if(i == 2)
        {
            std::string::size_type pos;
            if((pos = line.find('/')) != std::string::npos)
                m_httpVersion = line.substr(pos + 1);
        }
    }
}

void HttpRequest::parseArguments(const std::string &args)
{
    std::stringstream stream(args);
    std::string line;

    while(std::getline(stream, line, '&'))
    {
        std::string::size_type pos;
        if((pos = line.find('=')) == std::string::npos)
            m_urlArgs.push_back({{}, line});
        else
            m_urlArgs.push_back({line.substr(0, pos), line.substr(pos + 1)});
    }
}
