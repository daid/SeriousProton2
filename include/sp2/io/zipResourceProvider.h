#ifndef SP2_IO_ZIPRESOURCEPROVIDER_H
#define SP2_IO_ZIPRESOURCEPROVIDER_H

#include <sp2/io/resourceProvider.h>

namespace sp {
namespace io {

class ZipResourceProvider : public ResourceProvider
{
public:
    ZipResourceProvider(const string& zip_filename, int priority=0);
    
    virtual ResourceStreamPtr getStream(const string& filename) override;
    virtual std::chrono::system_clock::time_point getResourceModifyTime(const string& filename) override;
    virtual std::vector<string> findResources(const string& search_pattern) override;

private:
    string zip_filename;
    
    class ZipInfo
    {
    public:
        int method;
        uint64_t offset;
        uint64_t compressed_size;
        uint64_t uncompressed_size;
    };
    std::unordered_map<string, ZipInfo> contents;
};

}//namespace io
}//namespace sp

#endif//SP2_IO_ZIPRESOURCEPROVIDER_H
