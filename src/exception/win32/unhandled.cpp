#include <private/exception/win32/context.h>
#include <sp2/exception/unhandled.h>

#include <windows.h>
#include <psapi.h>
#include <dbghelp.h>

namespace sp {
namespace exception {

static LPTOP_LEVEL_EXCEPTION_FILTER next_unhandled_exception_filter;

static string exceptionCodeToString(int code)
{
    switch(code)
    {
    case EXCEPTION_ACCESS_VIOLATION: return "ACCESS_VIOLATION";
    case EXCEPTION_IN_PAGE_ERROR: return "IN_PAGE_ERROR";
    case EXCEPTION_INT_DIVIDE_BY_ZERO: return "INT_DIVIDE_BY_ZERO";
    }
    return "0x" + string::hex(code);
}

void unhandledExceptionHandler(const UnhandledExceptionInfo& info);

static LONG WINAPI win32UnhandledExceptionHandler(PEXCEPTION_POINTERS ex_info)
{
    UnhandledExceptionInfo info;
    
    Win32ExceptionContext context;
    context.input_context = ex_info->ContextRecord;
    
    info.base_report = "Exception: ";
    info.base_report += exceptionCodeToString(ex_info->ExceptionRecord->ExceptionCode);
    info.base_report += " at 0x";
    info.base_report += string::hex((int)ex_info->ExceptionRecord->ExceptionAddress);
    if (ex_info->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION || ex_info->ExceptionRecord->ExceptionCode == EXCEPTION_IN_PAGE_ERROR)
    {
        if (ex_info->ExceptionRecord->NumberParameters >= 2)
        {
            info.base_report += " while accessing 0x" + string::hex(ex_info->ExceptionRecord->ExceptionInformation[1]);
        }
    }
    
    Win32StackWalkContext stack_walk_context(context);
    do
    {
        AddressInfo address_info;
        stack_walk_context.fillAddressInfo(address_info);
        info.stack.push_back(address_info);
    }
    while(stack_walk_context.next());

    unhandledExceptionHandler(info);

    std::string message = info.base_report;
    message += "\n@" + info.stack[0].symbol;
    //Report the user that we crashed, instead of the default dialog.
    MessageBox(nullptr, message.c_str(), "Fatal exception...", MB_ICONSTOP);
    return EXCEPTION_EXECUTE_HANDLER;
}

void setupUnhandledExceptionHandler()
{
    next_unhandled_exception_filter = SetUnhandledExceptionFilter(win32UnhandledExceptionHandler);
}

};//namespace exception
};//namespace sp
