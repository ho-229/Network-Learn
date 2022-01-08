/**
 * @author Ho 229
 * @date 2021/7/26
 */

#include "url.h"
#include "httprequest.h"

std::unordered_set<std::string> HttpRequest::MethodSet
    {
        "GET",
        "POST",
        "HEAD",
        "DELETE"
    };

HttpRequest::HttpRequest()
{

}

void HttpRequest::parse()
{
    std::string::size_type offset = 0;
    const size_t size = m_rawData.size();

    if(m_rawData.empty() || !this->parseRequestLine(offset, m_rawData))
        return;

    // Parse headers
    while(offset < size && m_rawData.at(offset) != '\r')
    {
        std::pair<std::string_view, std::string_view> item;

        if(!Util::referTil(offset, item.first, m_rawData, [](const char &ch)
        { return ch == ':'; }))
            return;
        ++offset;

        // Ignore spaces
        while(offset < size && m_rawData.at(offset) == ' ')
            ++offset;

        if(!Util::referTil(offset, item.second, m_rawData, [](const char &ch)
        { return ch == '\r'; }))
            return;
        offset += 2;

        m_headers.emplace(item);
    }
    offset += 2;

    // Body
    if(offset < size)
        m_body = {m_rawData.data() + offset, size - offset};

    m_isValid = true;
}

void HttpRequest::reset()
{
    m_isValid = false;

    m_headers.clear();
    m_urlArguments.clear();

    m_rawData.clear();
}

std::string HttpRequest::uri() const
{
    if(!m_isValid)
        return {};

    return uriUnescape(std::move(std::string(m_uri)));
}

bool HttpRequest::parseRequestLine(std::string::size_type &offset,
                                   const std::string &data)
{
    // Method
    if(!Util::referTil(offset, m_method, data, [](const char &ch)
    { return ch == ' '; }))
        return false;
    ++offset;

    // URI
    if(Util::referTil(offset, m_uri, data, [](const char &ch)
    { return ch == '?' || ch == ' '; }))
    {
        if(data.at(offset) == '?')
        {
            std::string_view arg;

            do
            {
                ++offset;

                if(!Util::referTil(offset, arg, data, [](const char &ch)
                                   { return ch == '&' || ch == ' '; }))
                    return false;

                m_urlArguments.emplace_back(arg);
            }
            while(data[offset] == '&');
        }
    }
    else
        return false;
    ++offset;

    // Version
    if(!Util::referTil(offset, m_httpVersion, data, [](const char &ch)
    { return ch == '\r'; }))
        return false;
    offset += 2;

    return true;
}
