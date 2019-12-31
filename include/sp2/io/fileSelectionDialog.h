#ifndef SP2_IO_FILESELECTIONDIALOG_H
#define SP2_IO_FILESELECTIONDIALOG_H

#include <functional>
#include <sp2/string.h>

namespace sp {
namespace io {

/*
    Show an open file dialog and call the callback function as soon as a file is selected
    Note that this function blocks most OSes, but returns and calls the callback later on emscripten builds.
    Also, pressing cancel on the dialog does not give a callback.

    @param filter: A filter like ".zip", or an empty string for all files.
 */
void openFileDialog(const sp::string& extension_filter, std::function<void(const sp::string&)> callback);

}//namespace io
}//namespace sp

#endif//SP2_IO_FILESELECTIONDIALOG_H
