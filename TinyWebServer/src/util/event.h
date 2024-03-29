﻿/**
 * @author Ho 229
 * @date 2021/7/28
 */

#ifndef EVENT_H
#define EVENT_H

#include <functional>
#include <string_view>

#define STATIC_ALLOCATOR(Class) \
static void* operator new (size_t) \
{ \
static thread_local char space[sizeof(Class)]; \
return space; \
} \
static void operator delete (void *object, size_t) \
{ \
    static_cast<Class *>(object)->~Class(); \
}

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
    explicit Event() = default;
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
        ListenerError
    };

    explicit ExceptionEvent(const Error err, const std::string_view& message = {})
        : m_error(err), m_message(message) {}

    virtual Type type() const override { return Event::ExceptionEvent; }

    Error error() const { return m_error; }

    std::string_view message() const { return m_message; }

    STATIC_ALLOCATOR(ExceptionEvent)

private:
    const Error m_error;
    const std::string_view m_message;
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

    State state() const { return m_state; }

    STATIC_ALLOCATOR(ConnectEvent)

private:
    const AbstractSocket *m_socket = nullptr;
    const State m_state;
};

#endif // EVENT_H
