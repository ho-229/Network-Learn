/**
 * @author Ho 229
 * @date 2022/4/2
 */

#include "backtrace.h"

extern "C"
{
#ifdef _WIN32
# include <DbgHelp.h>
# pragma comment(lib, "dbghelp.lib")
#else
# include <string.h>
# include <signal.h>
# include <execinfo.h>
#endif
}

#ifdef _WIN32
LONG BackTrace::exceptionHandler(_EXCEPTION_POINTERS *exp)
{
    std::stringstream stream;
    void *buf[ADDR_MAX_NUM];

    stream << "\nEXCEPTION CODE: "
           << exp->ExceptionRecord->ExceptionCode;

    SymInitialize(GetCurrentProcess(), nullptr, true);

    uint16_t frames = CaptureStackBackTrace(0, ADDR_MAX_NUM, buf, nullptr);
    std::unique_ptr<SYMBOL_INFO> symbols(reinterpret_cast<SYMBOL_INFO *>(
        malloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char))));

    symbols->MaxNameLen   = 255;
    symbols->SizeOfStruct = sizeof(SYMBOL_INFO);

    stream << "\nNUMBER OF ADDRESSES: " << frames
           << "\n======= Stack Trace =======\n";

    for(uint16_t i = 0; i < frames; ++i)
    {
        SymFromAddr(GetCurrentProcess(),
                    reinterpret_cast<DWORD64>(buf[i]), nullptr, symbols.get());

        stream << i << ": [" << symbols->Index << "] " << symbols->Name
               << " Addr: " << symbols->Address
               << " Reg: " << symbols->Register << "\n";
    }

    stream << "\n";

    instance().m_handler(stream);

    return EXCEPTION_EXECUTE_HANDLER;
}
#else   // *nix
void BackTrace::signalHandler(int signum)
{
    std::stringstream stream;

    stream << "\nSIGNAL: " << strsignal(signum) << "\n";

    void *buf[ADDR_MAX_NUM] = {};

    int addrNum = backtrace(buf, ADDR_MAX_NUM);

    stream << "NUMBER OF ADDRESSES: " << addrNum
           << "\n======= Stack Trace =======\n";

    std::unique_ptr<char *> symbols(backtrace_symbols(buf, addrNum));
    if(!symbols)
    {
        stream << "BACKTRACE: CANNOT GET BACKTRACE SYMBOLS\n";
        exit(-2);
    }

    for(int i = 0; i < addrNum; ++i)
        stream << i << ": " << symbols.get()[i] << "\n";

    stream << "\n";

    instance().m_handler(stream);

    exit(-1);
}
#endif

BackTrace::BackTrace()
{
#ifdef _WIN32
    SetUnhandledExceptionFilter(exceptionHandler);
#else   // *nix
    signal(SIGSEGV, signalHandler);
    signal(SIGFPE, signalHandler);
#endif
}

void BackTrace::installHandler(const Handler &handler)
{
    instance().m_handler = handler;
}

BackTrace &BackTrace::instance()
{
    static /*thread_local*/ BackTrace backTrace;
    return backTrace;
}
