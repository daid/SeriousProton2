#ifndef SP2_IO_FILESYSTEM_H
#define SP2_IO_FILESYSTEM_H

#include <sp2/io/resourceProvider.h>

namespace sp {
namespace io {

bool isFile(const sp::string& path);
bool isDirectory(const sp::string& path);
/** Make a directory, including all base directories.
    Returns true when the directory was successfully made, or if the directory already exists */
bool makeDirectory(const sp::string& path);
sp::string dirname(const sp::string& path);
sp::string basename(const sp::string& path);

};//namespace io
};//namespace sp

#endif//SP2_IO_FILESYSTEM_H