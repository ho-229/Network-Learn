/**
 * @author Ho 229
 * @date 2021/7/26
 */

#include "url.h"
#include "httprequest.h"

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

    // Parse keep-alive
    const auto it = m_headers.find("Connection");
    m_isKeepAlive = it == m_headers.end() ?
                true : Util::strcasecmp(it->second, "close");
}

void HttpRequest::reset()
{
    m_isValid = false;

    m_headers.clear();
    m_urlArguments.clear();

    m_rawData.clear();
}

bool HttpRequest::parseRequestLine(std::string::size_type &offset,
                                   const std::string &data)
{
    // Method
    if(!Util::referTil<8>(offset, m_method, data, [](const char &ch)
    { return ch == ' '; }))
        return false;
    ++offset;

    // URI
    std::string_view uri;
    if(Util::referTil(offset, uri, data, [](const char &ch)
    { return ch == '?' || ch == ' '; }))
    {
        m_uri = std::move(uriUnescape(uri));

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
    if(!Util::referTil<10>(offset, m_httpVersion, data, [](const char &ch)
    { return ch == '\r'; }))
        return false;
    offset += 2;

    return true;
}
