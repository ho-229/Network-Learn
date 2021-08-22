/**
 * @author Ho 229
 * @date 2021/7/28
 */

#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <functional>

/**
 * @brief Abstract Event
 */
class Event
{
public:
    enum Type
    {
        ConnectEvent,
        ExceptionEvent
    };

    virtual ~Event() = default;

    virtual Type type() const = 0;

protected:
    explicit Event() {}
};

typedef std::function<void(Event *)> EventHandler;

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

    explicit ExceptionEvent(const Error err, const std::string& message = {})
        : m_error(err), m_message(message) {}

    virtual Type type() const override { return Event::ExceptionEvent; }

    Error error() const { return m_error; }

    std::string message() const { return m_message; }

private:
    const Error m_error;
    const std::string m_message;
};


/**
 * @brief Accept Event
 */

class AbstractSocket;

class ConnectEvent : public Event
{
public:
    enum State
    {
        Accpet,
        Close
    };

    explicit ConnectEvent(const AbstractSocket *socket, const State& state)
        : m_socket(socket), m_state(state)
    {}

    virtual Type type() const override { return Event::ConnectEvent; }

    const AbstractSocket *socket() const { return m_socket; }

    const State state() const { return m_state; }

private:
    const AbstractSocket *m_socket = nullptr;
    const State m_state;
};

#endif // EVENT_H
