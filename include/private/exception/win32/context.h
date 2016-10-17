#ifndef SP2_EXCEPTION_WINDOWS_CONTEXT_H
#define SP2_EXCEPTION_WINDOWS_CONTEXT_H

#include <sp2/exception/addressInfo.h>
#include <SFML/System/NonCopyable.hpp>

#include <windows.h>
#include <psapi.h>
#include <dbghelp.h>

namespace sp {
namespace exception {

class Win32ExceptionContext : sf::NonCopyable
{
public:
    Win32ExceptionContext();
    ~Win32ExceptionContext();
    
    HANDLE process;
    HANDLE thread;
    const CONTEXT* input_context;
};

class Win32StackWalkContext : sf::NonCopyable
{
public:
    Win32StackWalkContext(Win32ExceptionContext& exception_context);
    ~Win32StackWalkContext();

    bool next();
    void fillAddressInfo(AddressInfo& info);

    Win32ExceptionContext& exception_context;
    int machine_type;
    unsigned int previous_address;
    CONTEXT context;
#ifdef _WIN64
    WOW64_CONTEXT wow64context;
#endif
    PCONTEXT context_ptr;
    STACKFRAME64 stack_frame;
};

};//!namespace exception
};//!namespace sp

#endif//SP2_EXCEPTION_DUMMY_CONTEXT_H
