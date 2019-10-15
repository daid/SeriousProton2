#ifndef SP2_LAZY_LOADER_H
#define SP2_LAZY_LOADER_H

#include <sp2/io/resourceProvider.h>
#include <sp2/logging.h>
#include <sp2/threading/queue.h>
#include <thread>
#include <unordered_map>

namespace sp {
namespace io {

class LazyLoaderManager
{
private:
    static std::thread* thread;
    static sp::threading::Queue<std::function<void()>> queue;
    static void addWork(std::function<void()> f);
    
    template<class T> friend class LazyLoader;
};

template<class T> class LazyLoader : NonCopyable
{
public:
    T get(const string& name)
    {
        auto it = cached_items.find(name);
        if (it != cached_items.end())
        {
            return it->second;
        }
        
        T ptr = prepare(name);
        cached_items[name] = ptr;
        addToWorkqueue(ptr, name);
        return ptr;
    }

protected:
    void addToWorkqueue(T ptr, const string& name)
    {
        io::ResourceStreamPtr stream;
        if (name.find("#") > -1)
            stream = io::ResourceProvider::get(name.substr(0, name.find("#")));
        else
            stream = io::ResourceProvider::get(name);
        if (stream)
        {
            LazyLoaderManager::addWork([this, ptr, stream]()
            {
                this->backgroundLoader(ptr, stream);
            });
        }
        else
        {
            LOG(Warning, "Failed to load", name);
        }
    }

    virtual T prepare(const string& name) = 0;
    virtual void backgroundLoader(T ptr, io::ResourceStreamPtr stream) = 0;
private:
    std::unordered_map<string, T> cached_items;
};

}//namespace io
}//namespace sp


#endif//SP2_LAZY_LOADER_H
