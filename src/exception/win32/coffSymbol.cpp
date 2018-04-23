#include <private/exception/win32/context.h>
#include <sp2/logging.h>
#include <cxxabi.h>

namespace sp {
namespace exception {

void fillAddressInfoFromCoffSymbols(Win32ExceptionContext& exception_context, AddressInfo& info)
{
    char buffer[MAX_PATH];
    //No symbol found by dbghelp.dll from Microsoft.
    //We could be looking at a MINGW compiled module, look in the symbol table to find the matching symbol.
    strcpy(buffer, info.module.c_str());
    HANDLE file  = CreateFileA(buffer, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    HANDLE mapping = CreateFileMapping(file, NULL, PAGE_READONLY, 0, 0, NULL);
    const char* file_contents = (const char*)MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
    unsigned long filesize_hi;
    uint64_t filesize = GetFileSize(file, &filesize_hi);
    filesize |= uint64_t(filesize_hi) << 32;

    PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)file_contents;
    PIMAGE_NT_HEADERS nt_headers = (PIMAGE_NT_HEADERS)(file_contents + dos_header->e_lfanew);

    PIMAGE_SECTION_HEADER sections = (PIMAGE_SECTION_HEADER)((const char*)nt_headers + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) + nt_headers->FileHeader.SizeOfOptionalHeader);
    PIMAGE_SYMBOL symbol_table = (PIMAGE_SYMBOL)(file_contents + nt_headers->FileHeader.PointerToSymbolTable);
    const char* string_table = (const char*)&symbol_table[nt_headers->FileHeader.NumberOfSymbols];

    DWORD64 offset = 0;

    int text_section = -1;
    for(unsigned int section_index = 0; section_index < nt_headers->FileHeader.NumberOfSections; section_index++)
    {
        const char* section_name = (const char*)sections[section_index].Name;
        if (section_name[0] == '/')
            section_name = string_table + atoi((const char*)sections[section_index].Name + 1);
        if (strcmp(section_name, ".text") == 0)
        {
            text_section = section_index + 1;
            offset += sections[section_index].VirtualAddress;
            break;
        }
    }

    IMAGE_SYMBOL my_symbol;
    memset(&my_symbol, 0, sizeof(my_symbol));
    for(unsigned int symbol_index = 0; symbol_index < nt_headers->FileHeader.NumberOfSymbols; symbol_index++)
    {
        IMAGE_SYMBOL& symbol = symbol_table[symbol_index];
        if (symbol.SectionNumber == text_section)
        {
            if (ISFCN(symbol.Type))
            {
                if (my_symbol.Value < symbol.Value && symbol.Value + offset < (DWORD64)info.module_offset)
                    my_symbol = symbol;
            }
        }
        symbol_index += symbol.NumberOfAuxSymbols;
    }
    {
        if (my_symbol.N.Name.Short != 0)
        {
            memcpy(buffer, my_symbol.N.ShortName, 8);
            buffer[8] = '\0';
        }else{
            strcpy(buffer, string_table + my_symbol.N.Name.Long);
        }
        if (buffer[0] == '_')
        {
            int status = 0;
            char* demangled = abi::__cxa_demangle(buffer + 1, nullptr, nullptr, &status);
            if (demangled)
            {
                info.symbol = demangled;
                free(demangled);
            }else{
                info.symbol = buffer + 1;
            }
            info.symbol_offset = info.module_offset - (my_symbol.Value + offset);
        }
    }

    UnmapViewOfFile(file_contents);
    CloseHandle(mapping);
    CloseHandle(file);
}

};//namespace exception
};//namespace sp

