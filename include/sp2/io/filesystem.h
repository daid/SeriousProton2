#ifndef SP2_IO_FILESYSTEM_H
#define SP2_IO_FILESYSTEM_H

#include <sp2/io/resourceProvider.h>

namespace sp {
namespace io {

bool isFile(const string& path);
bool isDirectory(const string& path);
/** Make a directory, including all base directories.
    Returns true when the directory was successfully made, or if the directory already exists */
bool makeDirectory(const string& path);
string dirname(const string& path);
string basename(const string& path);
std::vector<string> listFiles(const string& path);

bool saveFileContents(const string& filename, const string& contents);
string loadFileContents(const string& filename);

// Get the path where configuration data can be stored.
const string& preferencePath();

}//namespace io
}//namespace sp

#endif//SP2_IO_FILESYSTEM_H
