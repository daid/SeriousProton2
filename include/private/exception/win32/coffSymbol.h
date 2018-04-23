#ifndef SP2_EXCEPTION_WINDOWS_COFF_SYMBOL_H
#define SP2_EXCEPTION_WINDOWS_COFF_SYMBOL_H

#include <sp2/exception/addressInfo.h>

namespace sp {
namespace exception {

void fillAddressInfoFromCoffSymbols(Win32ExceptionContext& exception_context, AddressInfo& info);

};//namespace exception
};//namespace sp

#endif//SP2_EXCEPTION_DUMMY_CONTEXT_H
