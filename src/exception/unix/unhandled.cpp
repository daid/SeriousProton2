#include <sp2/exception/unhandled.h>
#include <sp2/stringutil/convert.h>

#if !defined(ANDROID) && !defined(__EMSCRIPTEN__)

#include <sys/ucontext.h>
#include <ucontext.h>
#include <signal.h>
#include <execinfo.h>


namespace sp {
namespace exception {

void unhandledExceptionHandler(const UnhandledExceptionInfo& info);

static inline void fillBacktrace(UnhandledExceptionInfo& info, void* exception_position)
{
    void* traceback_buffer[32];
    int count = backtrace(traceback_buffer, 32);
    char** symbols = backtrace_symbols(traceback_buffer, count);
    bool active = exception_position == nullptr;
    for(int n=0; n<count; n++)
    {
        if (traceback_buffer[n] == exception_position)
            active = true;
        if (!active)
            continue;
        AddressInfo address_info;
        address_info.address = traceback_buffer[n];
        // symbols contains "module(symbol+0xoffset) [address]"
        address_info.module = symbols[n];
        address_info.symbol = address_info.module.substr(address_info.module.find("(") + 1);
        address_info.symbol_offset = stringutil::convert::toInt(address_info.symbol.substr(address_info.symbol.find("+") + 3, 16));
        address_info.symbol = address_info.symbol.substr(0, address_info.symbol.find("+"));
        address_info.module = address_info.module.substr(0, address_info.module.find("("));
        info.stack.push_back(address_info);
    }
}

static void unixTerminateHandler()
{
    UnhandledExceptionInfo info;

    info.base_report = "Unhandled exception";
    fillBacktrace(info, nullptr);
    unhandledExceptionHandler(info);

    std::abort();
}

static void unixSignalHandler(int signal, siginfo_t *si, void *context_ptr)
{
    ucontext_t *context = static_cast<ucontext_t*>(context_ptr);

    UnhandledExceptionInfo info;

    info.base_report = "Exception: ";
    switch(signal)
    {
    case SIGILL: info.base_report += "illigal instruction"; break;
    case SIGSEGV: info.base_report += "segmentation fault"; break;
    case SIGBUS: info.base_report += "bus error"; break;
    }
    void* exception_position = nullptr;
#ifdef REG_RIP
    exception_position = reinterpret_cast<void*>(context->uc_mcontext.gregs[REG_RIP]);
#elif defined(REG_EIP)
    exception_position = reinterpret_cast<void*>(context->uc_mcontext.gregs[REG_EIP]);
#elif defined(__arm__)
    exception_position = reinterpret_cast<void*>(context->uc_mcontext.arm_pc);
#endif
    info.base_report += " at 0x" + string::hex(reinterpret_cast<uintptr_t>(exception_position));
    info.base_report += " while accessing 0x" + string::hex(reinterpret_cast<uintptr_t>(si->si_addr));

    fillBacktrace(info, exception_position);
    unhandledExceptionHandler(info);

    exit(1);
}

void setupUnhandledExceptionHandler()
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(struct sigaction));
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = unixSignalHandler;
    sa.sa_flags = SA_SIGINFO;

    sigaction(SIGILL, &sa, nullptr);
    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGBUS, &sa, nullptr);
    sigaction(SIGTRAP, &sa, nullptr);

    std::set_terminate(unixTerminateHandler);
}

}//namespace exception
}//namespace sp

#endif//!ANDROID && !EMSCRIPTEN

