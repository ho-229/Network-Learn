﻿/**
 * @author Ho 229
 * @date 2021/7/26
 */

#ifndef HTTPSERVICES_H
#define HTTPSERVICES_H

#include <string>
#include <memory>
#include <filesystem>
#include <functional>
#include <unordered_map>

#include "httprequest.h"
#include "httpresponse.h"

namespace fs = std::filesystem;

typedef std::function<void(HttpRequest *, HttpResponse *)> Handler;
typedef std::unordered_map<std::string, std::shared_ptr<Handler>> MethodHandler;

class HttpServices
{
public:
    explicit HttpServices();

    void addService(const std::string &method, const std::string& uri,
                    const Handler& handler);

    void service(HttpRequest *httpRequest, HttpResponse* httpResponse) const;

    void setWorkDir(const fs::path& path);
    fs::path workDir() const { return m_workDir; }

private:
    std::unordered_map<std::string,       // URI
                       MethodHandler>     // Method -> Handler
        m_uris;

    fs::path m_workDir;
};

#endif // HTTPSERVICES_H
