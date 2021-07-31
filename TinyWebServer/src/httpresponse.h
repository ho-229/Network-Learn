/**
 * @author Ho 229
 * @date 2021/7/26
 */

#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <map>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

typedef std::pair<int, std::string> HttpState;

static std::map<std::string, std::string> PermissibleStaticType
    {
        {".html", "text/html"},
        {".css", "text/css"},
        {".js", "text/javascript"},
        {".png", "image/png"},
        {".jpg", "image/jpg"},
        {".jpeg", "image/jpeg"},
        {".gif", "image/gif"},
        {".svg", "image/svg+xml"},
        {".txt", "text/plain"},
        {".pdf", "application/pdf"},
        {".ico", "image/x-icon"},
        {".swf", "application/x-shockwave-flash"}
    };

class HttpResponse
{
public:
    enum BodyType
    {
        Normal,
        File
    };

    HttpResponse();

    void setText(const std::string text)
    {
        m_text = text;
        m_type = Normal;
    }
    std::string text() const { return m_text; }

    void setFilePath(const fs::path& path);
    fs::path filePath() const { return m_filePath; }

    void reset();

    bool isEmpty() const { return m_text.empty() && m_filePath.empty(); }

    BodyType bodyType() const { return m_type; }

    void setHttpState(const HttpState& state);
    HttpState httpState() const { return m_httpState; }

    void setRawHeader(const std::string& name, const std::string& value)
    { m_headers[name] = value; }
    std::string rawHeader(const std::string& name) const { return m_headers.at(name); }

    void toRawData(std::string& response);

    void buildErrorResponse(int state, const std::string& message);
    void buildFileResponse(const fs::path &filePath);

    static auto& PermissibleStaticTypes() { return PermissibleStaticType; }

private:
    std::pair<int, std::string> m_httpState = {200, "OK"};

    std::string m_text;

    std::map<std::string,   // Name
             std::string>   // Value
        m_headers;

    fs::path m_filePath;

    BodyType m_type = Normal;
};

#endif // HTTPRESPONSE_H
