/**
 * @author Ho 229
 * @date 2021/7/28
 */

#ifndef EVENT_H
#define EVENT_H

#include <string>

/**
 * @brief Abstract Event
 */
class Event
{
public:
    enum Type
    {
        AcceptEvent,
        ExceptionEvent
    };

    virtual ~Event() = default;

    virtual Type type() const = 0;

protected:
    explicit Event() {}
};

/**
 * @brief Exception Event
 */
class ExceptionEvent : public Event
{
public:
    enum Error
    {
        UnknownError,
        SocketLoadFailed,
        ListenFailed
    };

    explicit ExceptionEvent(Error err) : m_error(err) {}

    virtual Type type() const override { return Event::ExceptionEvent; }

    Error error() const { return m_error; }

private:
    const Error m_error;
};


/**
 * @brief Accept Event
 */
class AcceptEvent : public Event
{
public:
    explicit AcceptEvent(const std::string& hostName, const std::string& port)
        : m_hostName(hostName), m_port(port) {}

    virtual Type type() const override { return Event::AcceptEvent; }

    std::string hostName() const { return m_hostName; }
    std::string port() const { return m_port; }

private:
    const std::string m_hostName;
    const std::string m_port;
};

#endif // EVENT_H
