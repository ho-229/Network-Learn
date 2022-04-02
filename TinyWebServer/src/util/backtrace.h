#ifndef BACKTRACE_H
#define BACKTRACE_H

#include <memory>
#include <sstream>
#include <functional>

#ifdef _WIN32
# include <Windows.h>
#endif

#define ADDR_MAX_NUM 100

class BackTrace
{
public:
    using Handler = std::function<void(const std::stringstream &)>;

    explicit BackTrace();

    static void installHandler(const Handler &handler);

private:
    static BackTrace &instance();

    Handler m_handler;

#ifdef _WIN32
    static LONG WINAPI exceptionHandler(struct _EXCEPTION_POINTERS *exp);
#else
    static void signalHandler(int signum);
#endif
};

#endif // BACKTRACE_H
