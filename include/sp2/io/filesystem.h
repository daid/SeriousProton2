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
std::vector<sp::string> listFiles(const sp::string& path);

bool saveFileContents(const sp::string& filename, const sp::string& contents);
sp::string loadFileContents(const sp::string& filename);

// Get the path where configuration data can be stored.
//  Note that this caches the first call, if you change the application name mid run, it will still return the same path.
const sp::string& preferencePath(const sp::string& application_name);

}//namespace io
}//namespace sp

#endif//SP2_IO_FILESYSTEM_H
