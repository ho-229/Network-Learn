/**
 * @author Ho 229
 * @date 2021/10/5
 */

#ifndef ABSTRACTSERVICES_H
#define ABSTRACTSERVICES_H

#include <string>
#include <functional>

class HttpRequest;
class HttpResponse;
class AbstractSocket;

typedef std::function<void(HttpRequest *, HttpResponse *)> Handler;

class AbstractServices
{
public:
    virtual ~AbstractServices() = default;

    virtual void addService(const std::string& method, const std::string& uri,
                            const Handler& handler) = 0;

    virtual void setDefaultService(const std::string& method,
                                   const Handler& handler) = 0;

    virtual bool service(AbstractSocket *const socket) const = 0;

    void setMaxTimes(size_t num) { m_maxTimes = num; }
    size_t maxTimes() const { return m_maxTimes; }

protected:
    size_t m_maxTimes = 30;

    explicit AbstractServices() = default;

    // Disable copy
    AbstractServices(AbstractServices& other) = delete;
    AbstractServices& operator=(const AbstractServices& other) = delete;
};

#endif // ABSTRACTSERVICES_H
