#include <sp2/io/internalResourceProvider.h>
#include <string.h>

namespace sp {
namespace io {

class InternalResourceStream : public ResourceStream
{
private:
    const string& resource;
    unsigned int offset;
    
public:
    InternalResourceStream(const string& resource)
    : resource(resource)
    {
        offset = 0;
    }
    virtual ~InternalResourceStream()
    {
    }
    
    virtual sf::Int64 read(void* data, sf::Int64 size)
    {
        size = std::min(int(resource.size() - offset), int(size));
        memcpy(data, resource.data() + offset, size);
        offset += size;
        return size;
    }
    
    virtual sf::Int64 seek(sf::Int64 position)
    {
        offset = std::max(0, std::min(int(resource.size()), int(position)));
        return offset;
    }
    
    virtual sf::Int64 tell()
    {
        return offset;
    }
    
    virtual sf::Int64 getSize()
    {
        return resource.size();
    }
};


InternalResourceProvider::InternalResourceProvider(std::map<string, string>&& resources)
: resources(resources)
{
}

ResourceStreamPtr InternalResourceProvider::getStream(const string filename)
{
    if (!filename.startswith("internal:"))
        return nullptr;
    const auto& it = resources.find(filename.substr(9));
    if (it == resources.end())
        return nullptr;

    return std::make_shared<InternalResourceStream>(it->second);
}

std::vector<string> InternalResourceProvider::findResources(string search_pattern)
{
    std::vector<string> found_files;
    return found_files;
}

};//!namespace io
};//!namespace sp
