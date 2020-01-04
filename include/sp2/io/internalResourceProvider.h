#ifndef SP2_IO_INTERNALRESOURCEPROVIDER_H
#define SP2_IO_INTERNALRESOURCEPROVIDER_H

#include <sp2/io/resourceProvider.h>

namespace sp {
namespace io {

class InternalResourceProvider : public ResourceProvider
{
public:
    InternalResourceProvider(std::map<string, string>&& resources, int priority=0);
    
    virtual ResourceStreamPtr getStream(const string& filename) override;
    virtual std::vector<string> findResources(const string& search_pattern) override;

private:
    std::map<string, string> resources;
};

}//namespace io
}//namespace sp

#endif//SP2_IO_INTERNALRESOURCEPROVIDER_H
