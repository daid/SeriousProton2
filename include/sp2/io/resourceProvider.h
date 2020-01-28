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

    // Optional stream flags. Generally set by extra parameters on the filename.
    bool hasFlag(const string& name);
    string getFlag(const string& name);
protected:
    std::unordered_map<string, string> flags;

    friend class ResourceProvider;
};
typedef std::shared_ptr<ResourceStream> ResourceStreamPtr;

class ResourceProvider : public AutoPointerObject
{
public:
    ResourceProvider(int priority);
    
    virtual ResourceStreamPtr getStream(const string& filename) = 0;
    virtual std::chrono::system_clock::time_point getResourceModifyTime(const string& filename) { return std::chrono::system_clock::time_point(); }
    virtual std::vector<string> findResources(const string& search_pattern) = 0;

    static ResourceStreamPtr get(string filename);
    static std::chrono::system_clock::time_point getModifyTime(const string& filename);
    static std::vector<string> find(const string& search_pattern);

    //Create the default resource providers.
    //  The result of this depends on the OS and build.
    //  Usually this adds a directoryResourceProvider for the directroy "resources".
    //  But it can also create a zipResourceProvider for the running executable, for a single exe setup.
    //  And for android it will create a AndroidAssetResourceProvider that reads from android assets.
    static void createDefault();
protected:
    bool searchMatch(const string& name, const string& search_pattern);

private:
    int priority;
    static PList<ResourceProvider> providers;
};

}//namespace io
}//namespace sp

#endif//SP2_IO_RESOURCEPROVIDER_H
