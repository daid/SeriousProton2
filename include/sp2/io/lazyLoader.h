#ifndef SP2_LAZY_LOADER_H
#define SP2_LAZY_LOADER_H

#include <sp2/io/resourceProvider.h>
#include <sp2/logging.h>
#include <sp2/threading/queue.h>
#include <thread>
#include <map>

namespace sp {
namespace io {

class LazyLoaderManager
{
private:
    static std::thread* thread;
    static sp::threading::Queue<std::function<void()>> queue;
    static void addWork(std::function<void()> f);
    
    template<class T, class BackgroundLoaderDataType> friend class LazyLoader;
};

template<class T, class BackgroundLoaderDataType> class LazyLoader : sf::NonCopyable
{
public:
    T* get(string name)
    {
        {
            std::lock_guard<std::mutex> lock(mutex);
            auto it = cached_items.find(name);
            if (it != cached_items.end())
            {
                T* result = it->second;
                return result;
            }
            auto result_it = load_result_items.find(name);
            if (result_it != load_result_items.end())
            {
                BackgroundLoaderDataType* ptr = result_it->second;
                if (ptr)
                {
                    T* result = finalize(ptr);
                    cached_items[name] = result;
                    load_result_items.erase(result_it);
                    return result;
                }
                return fallback;
            }
            load_result_items[name] = nullptr;
        }
        
        BackgroundLoaderDataType* ptr = prepare(name);
        LazyLoaderManager::addWork([this, name, ptr]()
        {
            BackgroundLoaderDataType* p = this->backgroundLoader(ptr);
            std::lock_guard<std::mutex> lock(mutex);
            load_result_items[name] = p;
        });
        
        if (!fallback)
            fallback = loadFallback();
        return fallback;
    }

    void insert(string name, T* item)
    {
        cached_items[name] = item;
    }
protected:
    virtual BackgroundLoaderDataType* prepare(string name) = 0;
    virtual BackgroundLoaderDataType* backgroundLoader(BackgroundLoaderDataType* ptr) = 0;
    virtual T* finalize(BackgroundLoaderDataType* ptr) = 0;
    virtual T* loadFallback() = 0;
private:
    std::map<string, T*> cached_items;
    std::map<string, BackgroundLoaderDataType*> load_result_items;
    T* fallback;
    std::mutex mutex;
};

};//!namespace io
};//!namespace sp


#endif//SP2_LAZY_LOADER_H
