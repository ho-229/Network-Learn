/**
 * @author Ho 229
 * @date 2021/7/26
 */

#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <memory>
#include <variant>
#include <istream>

#include "../define.h"
#include "../util/headermap.h"

typedef std::pair<int, std::string> HttpState;

class HttpResponse
{
public:
    using StringBody = std::string;

    using StreamBody = std::unique_ptr<std::istream>;

    struct FileBody
    {
        File file;
        off_t offset;
        size_t count;
    };

    struct Body
    {
        enum Type
        {
            Text,
            Stream,
            File
        };

        Body(const StringBody &text = {}) : text(text), type(Text) {}
        Body(StreamBody &&stream) : stream(std::move(stream)), type(Stream) {}
        Body(const FileBody &file) : file(file), type(File) {}

        StringBody text;
        StreamBody stream;
        FileBody file = {};

        const Type type;
    };

    HttpResponse();

    void setBody(const StringBody &text);
    void setBody(StreamBody &&stream);
    void setBody(const FileBody &file);

    template <typename Visitor>
    decltype(auto) visitBody(Visitor &&visitor)
    { return visitBody(visitor, m_body); }

    template <typename Visitor>
    static decltype(auto) visitBody(Visitor &&visitor, Body &body)
    {
        if(body.type == Body::Stream)
            return visitor(body.stream);
        else if(body.type == Body::File)
            return visitor(body.file);

        return visitor(body.text);
    }

    void reset();

    bool isValid() const { return m_isValid; }

    void setKeepAlive(bool isKeepAlive);
    bool isKeepAlive() const { return m_isKeepAlive; }

    void setHttpState(const HttpState& state);
    HttpState httpState() const { return m_httpState; }

    template <bool isInsert = false>
    void setRawHeader(const std::string& name, const std::string& value)
    {
        if constexpr(isInsert)
                m_headers.insert({name, value});
        else
            m_headers[name] = value;
    }

    std::string rawHeader(const std::string& name) const
    {
        const auto it = m_headers.find(name);
        return it == m_headers.end() ? std::string() : it->second;
    }

    template<typename T>
    inline HttpResponse& operator<<(T &&body)
    {
        this->setBody(std::move(body));
        return *this;
    }

    static std::string matchContentType(const std::string &extension);

    static std::string replyRange(std::pair<size_t, size_t> range,
                                  const size_t total, size_t &offset, size_t &count);

private:
    friend class HttpServices;

    void toRawData(std::string& response);

    HttpState m_httpState = {200, "OK"};
    HeaderMap<std::string, std::string> m_headers;

    Body m_body;

    bool m_isValid = false;
    bool m_isKeepAlive = true;
};

#endif // HTTPRESPONSE_H
