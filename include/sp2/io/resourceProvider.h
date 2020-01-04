#ifndef SP2_IO_RESOURCEPROVIDER_H
#define SP2_IO_RESOURCEPROVIDER_H

#include <sp2/string.h>
#include <sp2/pointer.h>
#include <sp2/pointerList.h>
#include <memory>
#include <chrono>


namespace sp {
namespace io {

class ResourceStream : public sp::NonCopyable
{
public:
    virtual ~ResourceStream() {}

    virtual int64_t read(void* data, int64_t size) = 0;
    virtual int64_t seek(int64_t position) = 0;
    virtual int64_t tell() = 0;
    virtual int64_t getSize() = 0;
    
    string readLine();
    string readAll();
};
typedef std::shared_ptr<ResourceStream> ResourceStreamPtr;

class ResourceProvider : public AutoPointerObject
{
public:
    ResourceProvider(int priority);
    
    virtual ResourceStreamPtr getStream(const string& filename) = 0;
    virtual std::chrono::system_clock::time_point getResourceModifyTime(const string& filename) { return std::chrono::system_clock::time_point(); }
    virtual std::vector<string> findResources(const string& search_pattern) = 0;

    static ResourceStreamPtr get(const string& filename);
    static std::chrono::system_clock::time_point getModifyTime(const string& filename);
    static std::vector<string> find(const string& search_pattern);
protected:
    bool searchMatch(const string& name, const string& search_pattern);

private:
    int priority;
    static PList<ResourceProvider> providers;
};

}//namespace io
}//namespace sp

#endif//SP2_IO_RESOURCEPROVIDER_H
