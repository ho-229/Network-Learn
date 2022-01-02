/**
 * @author Ho 229
 * @date 2021/7/26
 */

#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <memory>
#include <istream>

#include "../define.h"
#include "../util/headermap.h"

typedef std::pair<int, std::string> HttpState;

class HttpResponse
{
public:
    enum class BodyType
    {
        PlainText,
        File,
        Stream
    };

    HttpResponse();

    void setText(const std::string& text);
    std::string text() const { return m_text; }

    bool sendStream(std::unique_ptr<std::istream> &&stream, size_t count = 0);

    void sendFile(File fd, off_t offset, size_t count);

    void reset();

    bool isEmpty() const { return m_text.empty() && !m_stream; }

    BodyType bodyType() const { return m_type; }

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

    void toRawData(std::string& response);

    inline HttpResponse& operator<<(const std::string& text)
    {
        m_text.append(text);
        m_headers["Content-Length"] = std::to_string(m_text.size());

        m_type = BodyType::PlainText;

        return *this;
    }

    static std::unordered_map<std::string, std::string> PermissibleStaticTypes;

    static inline std::string matchContentType(const std::string &extension)
    {
        const auto it = PermissibleStaticTypes.find(extension);
        return it == PermissibleStaticTypes.end() ? std::string() : it->second;
    }

private:
    friend class HttpServices;

    HttpState m_httpState = {200, "OK"};

    std::string m_text;                         // Text
    std::unique_ptr<std::istream> m_stream;     // Stream

    struct
    {
        File file;
        off_t offset;
    } m_file;                                   // File

    size_t m_count;

    HeaderMap m_headers;

    BodyType m_type = BodyType::PlainText;
};

#endif // HTTPRESPONSE_H
