#include <sp2/io/clipboard.h>
#include <sp2/logging.h>

#include <SDL_clipboard.h>

namespace sp {
namespace io {

string Clipboard::get()
{
    return SDL_GetClipboardText();
}

void Clipboard::set(const string& value)
{
    SDL_SetClipboardText(value.c_str());
}

}//namespace sp
}//namespace io
