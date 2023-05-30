#ifndef SP2_IO_DIRECTORYRESOURCEPROVIDER_H
#define SP2_IO_DIRECTORYRESOURCEPROVIDER_H

#include <sp2/io/resourceProvider.h>

namespace sp {
namespace io {

class FileResourceStream : public ResourceStream
{
private:
    FILE* f;
    int64_t size;
public:
    FileResourceStream(const string& filename);
    virtual ~FileResourceStream();
    bool isOpen();
    virtual int64_t read(void* data, int64_t size) override;
    virtual int64_t seek(int64_t position) override;
    virtual int64_t tell() override;
    virtual int64_t getSize() override;
};

class DirectoryResourceProvider : public ResourceProvider
{
public:
    DirectoryResourceProvider(const string& base_path, int priority=0);
    
    virtual ResourceStreamPtr getStream(const string& filename) override;
    virtual std::chrono::system_clock::time_point getResourceModifyTime(const string& filename) override;
    virtual std::vector<string> findResources(const string& search_pattern) override;

private:
    string base_path;
    
    void findResources(std::vector<string>& found_files, const string& path, const string& search_pattern);
};

}//namespace io
}//namespace sp

#endif//SP2_IO_DIRECTORYRESOURCEPROVIDER_H
