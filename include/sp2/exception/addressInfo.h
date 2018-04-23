#ifndef SP2_EXCEPTION_ADDRESSINFO_H
#define SP2_EXCEPTION_ADDRESSINFO_H

#include <sp2/string.h>

namespace sp {
namespace exception {

class AddressInfo
{
public:
    AddressInfo()
    {
        address = nullptr;
        module_offset = -1;
        symbol_offset = -1;
        source_line = -1;
    }

    void* address;
    string module;
    int module_offset;
    string symbol;
    int symbol_offset;

    string source_file;
    int source_line;
};

};//namespace exception
};//namespace sp

#endif//SP2_EXCEPTION_ADDRESSINFO_H
