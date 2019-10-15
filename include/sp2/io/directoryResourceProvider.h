#ifndef SP2_IO_DIRECTORYRESOURCEPROVIDER_H
#define SP2_IO_DIRECTORYRESOURCEPROVIDER_H

#include <sp2/io/resourceProvider.h>

namespace sp {
namespace io {

class DirectoryResourceProvider : public ResourceProvider
{
public:
    DirectoryResourceProvider(const string& base_path);
    
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
