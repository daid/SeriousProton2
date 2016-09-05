#include <sp2/io/clipboard.h>
#include <sp2/logging.h>

#ifdef __WIN32__
#include <sp2/window.h>
#include <windows.h>
#endif
#ifdef __linux__
#include <stdio.h>
#endif

namespace sp {
namespace io {

string Clipboard::virtual_clipboard;

string Clipboard::get()
{
#ifdef __WIN32__
    if (!Window::window)
        return "";

    if (!OpenClipboard(Window::window->render_window.getSystemHandle()))
    {
        LOG(Warning, "Failed to open the clipboard for reading");
        return "";
    }
    HANDLE handle = GetClipboardData(CF_TEXT);
    if (!handle)
    {
        LOG(Warning, "Failed to open the clipboard for reading");
        CloseClipboard();
        return "";
    }
    string ret;
    ret = static_cast<char*>(GlobalLock(handle));
    GlobalUnlock(handle);
    CloseClipboard();
    return ret;
#endif//__WIN32__
#ifdef __linux__
    FILE* pipe = popen("/usr/bin/xclip -o -selection clipboard", "r");
    if (!pipe)
    {
        LOG(Warning, "Failed to execute /usr/bin/xclip for clipboard access");
        return "";
    }
    char buffer[1024];
    std::string result = "";
    while (!feof(pipe))
    {
        if (fgets(buffer, 1024, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);
    return result;
#endif

    return virtual_clipboard;
}

void Clipboard::set(string value)
{
#ifdef __WIN32__
    if (!OpenClipboard(Window::window->render_window.getSystemHandle()))
    {
        LOG(Warning, "Failed to open the clipboard for writing");
        return;
    }

    EmptyClipboard();

    HANDLE string_handle;
    size_t string_size = (value.length()+1) * sizeof(char);
    string_handle = GlobalAlloc(GMEM_MOVEABLE, string_size);

    if (!string_handle)
    {
        LOG(Warning, "Failed to allocate a string for the clipboard writing");
        CloseClipboard();
        return;
    }

    memcpy(GlobalLock(string_handle), value.c_str(), string_size);
    GlobalUnlock(string_handle);
    SetClipboardData(CF_TEXT, string_handle);

    CloseClipboard();
#endif//__WIN32__
#ifdef __linux__
    FILE* pipe = popen("/usr/bin/xclip -i -selection clipboard -silent", "we");
    if (!pipe)
    {
        LOG(Warning, "Failed to execute /usr/bin/xclip for clipboard access");
        return;
    }
    fwrite(value.c_str(), value.size(), 1, pipe);
    pclose(pipe);
#endif
    virtual_clipboard = value;
}

};//!namespace sp
};//!namespace io
