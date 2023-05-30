#include <sp2/io/directoryResourceProvider.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <SDL.h>

namespace sp {
namespace io {

FileResourceStream::FileResourceStream(const string& filename)
{
    f = fopen(filename.c_str(), "rb");
    size = -1;
}

FileResourceStream::~FileResourceStream()
{
    if (f)
        fclose(f);
}

bool FileResourceStream::isOpen()
{
    return f != nullptr;
}

int64_t FileResourceStream::read(void* data, int64_t size)
{
    return fread(data, 1, size, f);
}

int64_t FileResourceStream::seek(int64_t position)
{
    fseek(f, position, SEEK_SET);
    return tell();
}

int64_t FileResourceStream::tell()
{
    return ftell(f);
}

int64_t FileResourceStream::getSize()
{
    if (size == -1)
    {
        int64_t pos = tell();
        fseek(f, 0, SEEK_END);
        size = tell();
        seek(pos);
    }
    return size;
}


DirectoryResourceProvider::DirectoryResourceProvider(const string& base_path, int priority)
: ResourceProvider(priority), base_path(base_path)
{
}

ResourceStreamPtr DirectoryResourceProvider::getStream(const string& filename)
{
    std::shared_ptr<FileResourceStream> stream = std::make_shared<FileResourceStream>(base_path + "/" + filename);
    if (stream->isOpen())
        return stream;
    return nullptr;
}

std::chrono::system_clock::time_point DirectoryResourceProvider::getResourceModifyTime(const string& filename)
{
    struct stat stat_info;
    if (stat((base_path + "/" + filename).c_str(), &stat_info) != 0)
        return std::chrono::system_clock::time_point();
    return std::chrono::system_clock::from_time_t(stat_info.st_mtime);
}

std::vector<string> DirectoryResourceProvider::findResources(const string& search_pattern)
{
    std::vector<string> found_files;
    findResources(found_files, "", search_pattern);
    return found_files;
}

void DirectoryResourceProvider::findResources(std::vector<string>& found_files, const string& path, const string& search_pattern)
{
    DIR* dir = opendir((base_path + "/" + path).c_str());
    if (!dir)
        return;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        if (entry->d_name[0] == '.')
            continue;
        string name = path + string(entry->d_name);
        if (searchMatch(name, search_pattern))
            found_files.push_back(name);
        findResources(found_files, path + string(entry->d_name) + "/", search_pattern);
    }
    closedir(dir);
}

}//namespace io
}//namespace sp
