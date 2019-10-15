#ifndef SP2_IO_CLIPBOARD_H
#define SP2_IO_CLIPBOARD_H

#include <sp2/string.h>

namespace sp {
namespace io {

class Clipboard
{
public:
    static string get();
    static void set(const string& value);

private:
    static string virtual_clipboard;
};

}//namespace io
}//namespace sp

#endif//CLIPBOARD_H
