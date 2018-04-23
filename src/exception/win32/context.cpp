#include <private/exception/win32/context.h>
#include <private/exception/win32/coffSymbol.h>

namespace sp {
namespace exception {

Win32ExceptionContext::Win32ExceptionContext()
{
    input_context = nullptr;
    process = GetCurrentProcess();
    thread = GetCurrentThread();
    SymInitialize(process, nullptr, true);
}

Win32ExceptionContext::~Win32ExceptionContext()
{
    SymCleanup(process);
}

Win32StackWalkContext::Win32StackWalkContext(Win32ExceptionContext& exception_context)
: exception_context(exception_context)
{
    ZeroMemory(&context, sizeof(context));
    context.ContextFlags = CONTEXT_FULL;

    ZeroMemory(&stack_frame, sizeof(stack_frame));

#ifdef _WIN64
    BOOL wow64 = FALSE;
    IsWow64Process(hProcess, &wow64);
    if (wow64)
    {
        machine_type = IMAGE_FILE_MACHINE_I386;
        ZeroMemory(&wow64context, sizeof(wow64context));
        wow64context.ContextFlags = WOW64_CONTEXT_FULL;
        if (!Wow64GetThreadContext(thread, &wow64context))
            return;
        context_ptr = (PCONTEXT)&wow64context;
        stack_frame.AddrPC.Offset = wow64context.Eip;
        stack_frame.AddrPC.Mode = AddrModeFlat;
        stack_frame.AddrStack.Offset = wow64context.Esp;
        stack_frame.AddrStack.Mode = AddrModeFlat;
        stack_frame.AddrFrame.Offset = wow64context.Ebp;
        stack_frame.AddrFrame.Mode = AddrModeFlat;
    }
    else
#endif
    {
        if (exception_context.input_context)
        {
            context = *exception_context.input_context;
        }
        else
        {
            if (!GetThreadContext(exception_context.thread, &context))
            {
                return;
            }
        }
        context_ptr = &context;

#ifdef _WIN64
        machine_type = IMAGE_FILE_MACHINE_AMD64;
        stack_frame.AddrPC.Offset = context_ptr->Rip;
        stack_frame.AddrPC.Mode = AddrModeFlat;
        stack_frame.AddrStack.Offset = context_ptr->Rsp;
        stack_frame.AddrStack.Mode = AddrModeFlat;
        stack_frame.AddrFrame.Offset = context_ptr->Rbp;
        stack_frame.AddrFrame.Mode = AddrModeFlat;
#else
        machine_type = IMAGE_FILE_MACHINE_I386;
        stack_frame.AddrPC.Offset = context_ptr->Eip;
        stack_frame.AddrPC.Mode = AddrModeFlat;
        stack_frame.AddrStack.Offset = context_ptr->Esp;
        stack_frame.AddrStack.Mode = AddrModeFlat;
        stack_frame.AddrFrame.Offset = context_ptr->Ebp;
        stack_frame.AddrFrame.Mode = AddrModeFlat;
#endif
    }

    previous_address = stack_frame.AddrStack.Offset - 1;
    
    if (!StackWalk64(machine_type, exception_context.process, exception_context.thread, &stack_frame, context_ptr, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
    {
        stack_frame.AddrStack.Offset = 0;
    }
}

Win32StackWalkContext::~Win32StackWalkContext()
{
}

bool Win32StackWalkContext::next()
{
    if (stack_frame.AddrStack.Offset <= previous_address || stack_frame.AddrPC.Offset == 0xBAADF00D)
        return false;
    previous_address = stack_frame.AddrStack.Offset;
    if (stack_frame.AddrFrame.Offset == 0)
        return false;
    if (!StackWalk64(machine_type, exception_context.process, exception_context.thread, &stack_frame, context_ptr, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
        return false;
    return true;
}

void Win32StackWalkContext::fillAddressInfo(AddressInfo& info)
{
    char buffer[MAX_PATH];
    
    info.address = (void*)stack_frame.AddrPC.Offset;
    
    HMODULE module = (HMODULE)(INT_PTR)SymGetModuleBase64(exception_context.process, (DWORD64)(INT_PTR)info.address);
    if (module)
    {
        info.module_offset = (DWORD64)info.address - (DWORD64)module;
        if (GetModuleFileNameExA(exception_context.process, module, buffer, sizeof(buffer)))
        {
            info.module = buffer;
            
            DWORD64 displacement = 0;
            PSYMBOL_INFO symbol_info = (PSYMBOL_INFO)buffer;
            symbol_info->SizeOfStruct = sizeof(SYMBOL_INFO);
            symbol_info->MaxNameLen = MAX_PATH - sizeof(SYMBOL_INFO);
            if (SymFromAddr(exception_context.process, (DWORD64)info.address, &displacement, symbol_info))
            {
                info.symbol = symbol_info->Name;
                info.symbol_offset = displacement;
                
                DWORD displacement = 0;
                IMAGEHLP_LINE64 line_info;
                memset(&line_info, 0, sizeof(line_info));
                if (SymGetLineFromAddr64(exception_context.process, (DWORD64)info.address, &displacement, &line_info))
                {
                    info.source_file = line_info.FileName;
                    info.source_line = line_info.LineNumber;
                }
            }
            else
            {
                fillAddressInfoFromCoffSymbols(exception_context, info);
            }
        }
    }
}

};//namespace exception
};//namespace sp
