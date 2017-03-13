#ifndef SP2_IO_DIRECTORYRESOURCEPROVIDER_H
#define SP2_IO_DIRECTORYRESOURCEPROVIDER_H

#include <sp2/io/resourceProvider.h>

namespace sp {
namespace io {

class DirectoryResourceProvider : public ResourceProvider
{
public:
    DirectoryResourceProvider(string base_path);
    
    virtual ResourceStreamPtr getStream(const string filename) override;
    virtual sf::Time getFileModifyTime(const string filename) override;
    virtual std::vector<string> findFilenames(const string search_pattern) override;

private:
    string base_path;
    
    void findResources(std::vector<string>& found_files, const string path, const string search_pattern);
};

};//!namespace io
};//!namespace sp

#endif//RESOURCELOADER_H
