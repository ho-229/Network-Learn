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
    this->reset();

    std::string::size_type offset = 0;
    const size_t size = data.size();

    if(data.empty() || !this->parseRequestLine(offset, data))
        return;

    // Parse headers
    std::string key, value;
    while(offset < size && data.at(offset) != '\r')
    {
        if(!Util::copyTil(offset, key, data, ':'))
            return;
        ++offset;

        while(offset < size && data.at(offset) == ' ')
            ++offset;

        if(!Util::copyTil(offset, value, data, '\r'))
            return;
        offset += 2;

        m_headers.insert({key, value});

        key.clear();
        value.clear();
    }
    offset += 2;

    // Body
    if(offset < size)
        m_body.assign(data.c_str() + offset, size - offset);

    m_isValid = true;
}

void HttpRequest::reset()
{
    m_method.clear();
    m_uri.clear();
    m_httpVersion.clear();
    m_body.clear();

    m_headers.clear();
    m_urlArgs.clear();

    m_isValid = false;
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

bool HttpRequest::parseRequestLine(std::string::size_type &offset,
                                   const std::string &data)
{
    // Method
    if(!Util::copyTil<7>(offset, m_method, data, ' '))
        return false;
    ++offset;

    // URI
    if(Util::copyTil(offset, m_uri, data,
                      [](const char &ch) -> bool { return ch == '?' || ch == ' '; }))
    {
        if(data.at(offset) == '?')
        {
            std::string args;
            ++offset;

            if(!Util::copyTil(offset, args, data, ' '))
                return false;
            this->parseArguments(args);
        }
    }
    else
        return false;
    ++offset;

    // Version
    if(!Util::copyTil(offset, m_httpVersion, data, '\r'))
        return false;
    offset += 2;

    return true;
}

void HttpRequest::parseArguments(const std::string &args)
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
