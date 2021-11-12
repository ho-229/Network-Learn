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
    if(data.empty())
        return;

    this->parse(data);
}

void HttpRequest::parse(const std::string &data)
{
    m_headers.clear();
    m_urlArgs.clear();
    m_isValid = false;

    std::string::size_type offset = 0;

    const auto requestLine = Util::getLine(offset, data, "\r\n");
    if(requestLine.empty() || !this->parseRequestLine(std::string(requestLine)))
        return;

    std::string line;
    while(true)
    {
        line = Util::getLine(offset, data, "\r\n");
        if(line.empty())
            break;

        Util::toLower(line);

        const auto splitPos = line.find(':');
        if(splitPos == std::string_view::npos)
            break;

        // Remove spaces forward
        auto keyEnd = splitPos;
        while(keyEnd - 1 > 0 && line[keyEnd] == ' ')
            --keyEnd;

        // Remove spaces backward
        auto valueStart = splitPos + 1;
        while(valueStart + 1 < line.size() && line[valueStart] == ' ')
            ++valueStart;

        m_headers[line.substr(0, keyEnd)] = line.substr(valueStart);
    }

    m_body = data.substr(offset);

    m_isValid = true;
}

std::pair<int64_t, int64_t> HttpRequest::range() const
{
    const auto it = m_headers.find("range");

    if(it == m_headers.end())
        return {0, 0};

    const std::regex express("bytes=(\\d+)-(\\d+)");
    std::smatch result;

    if(!std::regex_match(it->second, result, express))
        return {0, 0};

    return {atoll(result[1].str().c_str()), atoll(result[2].str().c_str())};
}

bool HttpRequest::parseRequestLine(const std::string &data)
{
    std::string::size_type offset = 0;

    // Method
    const auto method = Util::getLine(offset, data, " ");
    if(method.empty())
        return false;
    else
        m_method = method;

    // URI
    const auto uri = Util::getLine(offset, data, " ");
    if(uri.empty())
        return false;
    else
    {
        std::string::size_type pos;

        while(true)
        {
            if((pos = uri.find('?')) == std::string::npos)
                m_uri = uri;
            else
            {
                if(uri[pos - 1] == '\\')   // Ignore "\?"
                    continue;

                m_uri = uri.substr(0, pos);
                this->parseArguments(uri.substr(pos + 1));
            }

            break;
        }
    }

    // Version
    const auto version = Util::getLine(offset, data, " ");
    if(version.empty())
        return false;
    else
    {
        std::string::size_type pos;
        if((m_isValid = ((pos = version.find('/')) != std::string::npos)))
            m_httpVersion = version.substr(pos + 1);
    }

    return true;
}

void HttpRequest::parseArguments(const std::string_view &args)
{
    std::string::size_type offset = 0;
    std::string_view line;

    while(!(line = Util::getLine(offset, args, "&")).empty())
    {
        std::string::size_type pos;
        if((pos = line.find('=')) == std::string::npos)
            m_urlArgs.push_back({{}, std::string(line)});
        else
            m_urlArgs.push_back({std::string(line.substr(0, pos)),
                                 std::string(line.substr(pos + 1))});
    }
}
