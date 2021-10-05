/**
 * @author Ho 229
 * @date 2021/7/26
 */

#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <map>
#include <string>
#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;

typedef std::pair<int, std::string> HttpState;

static std::unordered_map<std::string, std::string> PermissibleStaticTypes
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
        PlainText,
        File
    };

    HttpResponse();

    void setText(const std::string text);
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

    static auto& permissibleStaticTypes() { return PermissibleStaticTypes; }

private:
    std::pair<int, std::string> m_httpState = {200, "OK"};

    inline void initializatHeaders();

    std::string m_text;

    std::map<std::string,   // Name
             std::string>   // Value
        m_headers;

    fs::path m_filePath;

    BodyType m_type = PlainText;
};

#endif // HTTPRESPONSE_H
