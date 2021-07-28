/**
 * @author Ho 229
 * @date 2021/7/26
 */

#include "httprequest.h"

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
    std::regex express("(\\w+)\\s(.+)\\?(.+)\\sHTTP");
    std::smatch result;

    if(std::regex_search(data, result, express))
    {
        int i = 0;
        for(const auto& it : result)
        {
            switch(i)
            {
            case 1:
                m_method = it.str();
                break;
            case 2:
                m_uri = it.str();
                break;
            case 3:
                this->buildArgs(it.str());
                break;
            }
            ++i;
        }
    }
}

void HttpRequest::buildArgs(const std::string &args)
{
    std::regex express("&+");

    std::vector<std::string> result(
        std::sregex_token_iterator(args.begin(), args.end(), express, -1),
        std::sregex_token_iterator());

    for(const std::string& it : result)
    {
        if(it.find('=') == std::string::npos)   // Not found "<key>=<value>"
            m_urlArgs.push_back({{}, it});
        else
        {
            std::regex express("(.*)=(.*)");
            std::smatch result;
            if(std::regex_search(it, result, express))
                m_urlArgs.push_back({result[1], result[2]});
        }
    }
}
