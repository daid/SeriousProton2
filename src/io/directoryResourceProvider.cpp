#include <sp2/io/directoryResourceProvider.h>
#include <SFML/System/FileInputStream.hpp>
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>

namespace sp {
namespace io {

class FileResourceStream : public ResourceStream
{
    sf::FileInputStream stream;
    bool open_success;
public:
    FileResourceStream(string filename)
    {
        open_success = stream.open(filename);
    }
    virtual ~FileResourceStream()
    {
    }
    
    bool isOpen()
    {
        return open_success;
    }
    
    virtual sf::Int64 read(void* data, sf::Int64 size)
    {
        return stream.read(data, size);
    }
    virtual sf::Int64 seek(sf::Int64 position)
    {
        return stream.seek(position);
    }
    virtual sf::Int64 tell()
    {
        return stream.tell();
    }
    virtual sf::Int64 getSize()
    {
        return stream.getSize();
    }
};


DirectoryResourceProvider::DirectoryResourceProvider(string base_path)
: base_path(base_path)
{
}

ResourceStreamPtr DirectoryResourceProvider::getStream(const string filename)
{
    std::shared_ptr<FileResourceStream> stream = std::make_shared<FileResourceStream>(base_path + "/" + filename);
    if (stream->isOpen())
        return stream;
    return nullptr;
}

std::chrono::system_clock::time_point DirectoryResourceProvider::getResourceModifyTime(const string filename)
{
    struct stat stat_info;
    if (stat((base_path + "/" + filename).c_str(), &stat_info) != 0)
        return std::chrono::system_clock::time_point();
    return std::chrono::system_clock::from_time_t(stat_info.st_mtime);
}

std::vector<string> DirectoryResourceProvider::findResources(string search_pattern)
{
    std::vector<string> found_files;
    findResources(found_files, "", search_pattern);
    return found_files;
}

void DirectoryResourceProvider::findResources(std::vector<string>& found_files, const string path, const string search_pattern)
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

};//namespace io
};//namespace sp
