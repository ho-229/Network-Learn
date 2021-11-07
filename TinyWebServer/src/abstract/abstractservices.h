/**
 * @author Ho 229
 * @date 2021/10/5
 */

#ifndef ABSTRACTSERVICES_H
#define ABSTRACTSERVICES_H

#include <string>
#include <functional>

class AbstractSocket;

class AbstractServices
{
public:
    virtual ~AbstractServices() = default;

    virtual bool process(AbstractSocket *const socket) const = 0;

    /**
     * @brief Maximum number of requests per connection
     * @note If num == 0 does not limit the number of requests
     * @default 0
     */
    void setMaxTimes(size_t num) { m_maxTimes = num; }
    size_t maxTimes() const { return m_maxTimes; }

protected:
    size_t m_maxTimes = 0;

    explicit AbstractServices() = default;

    // Disable copy
    AbstractServices(AbstractServices& other) = delete;
    AbstractServices& operator=(const AbstractServices& other) = delete;
};

#endif // ABSTRACTSERVICES_H
