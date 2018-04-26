#ifndef SP2_IO_RESOURCEPROVIDER_H
#define SP2_IO_RESOURCEPROVIDER_H

#include <sp2/string.h>
#include <sp2/pointer.h>
#include <sp2/pointerList.h>
#include <SFML/System/InputStream.hpp>
#include <memory>
#include <chrono>


namespace sp {
namespace io {

class ResourceStream : public sf::InputStream
{
public:
    virtual ~ResourceStream() {}
    
    string readLine();
    string readAll();
};
typedef std::shared_ptr<ResourceStream> ResourceStreamPtr;

class ResourceProvider : public AutoPointerObject
{
public:
    ResourceProvider();
    
    virtual ResourceStreamPtr getStream(const string filename) = 0;
    virtual std::chrono::system_clock::time_point getResourceModifyTime(const string filename) { return std::chrono::system_clock::time_point(); }
    virtual std::vector<string> findResources(const string search_pattern) = 0;

    static ResourceStreamPtr get(const string filename);
    static std::chrono::system_clock::time_point getModifyTime(const string filename);
    static std::vector<string> find(const string search_pattern);
protected:
    bool searchMatch(const string name, const string search_pattern);

private:
    static PList<ResourceProvider> providers;
};

};//namespace io
};//namespace sp

#endif//SP2_IO_RESOURCEPROVIDER_H
