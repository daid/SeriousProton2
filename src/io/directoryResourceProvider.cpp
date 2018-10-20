#include <sp2/io/directoryResourceProvider.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>

namespace sp {
namespace io {

class FileResourceStream : public ResourceStream
{
private:
    FILE* f;
    int64_t size;
public:
    FileResourceStream(string filename)
    {
        f = fopen(filename.c_str(), "rb");
        size = -1;
    }
    virtual ~FileResourceStream()
    {
        if (f)
            fclose(f);
    }
    
    bool isOpen()
    {
        return f != nullptr;
    }
    
    virtual int64_t read(void* data, int64_t size) override
    {
        return fread(data, 1, size, f);
    }
    virtual int64_t seek(int64_t position) override
    {
        fseek(f, position, SEEK_SET);
        return tell();
    }
    virtual int64_t tell() override
    {
        return ftell(f);
    }
    virtual int64_t getSize() override
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
