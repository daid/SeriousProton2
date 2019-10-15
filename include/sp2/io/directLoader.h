#ifndef SP2_IO_DIRECT_LOADER_H
#define SP2_IO_DIRECT_LOADER_H

#include <sp2/io/resourceProvider.h>
#include <sp2/logging.h>
#include <sp2/threading/queue.h>
#include <thread>
#include <unordered_map>

namespace sp {
namespace io {

template<class T> class DirectLoader : NonCopyable
{
public:
    T get(const string& name)
    {
        auto it = cached_items.find(name);
        if (it != cached_items.end())
        {
            return it->second;
        }
        
        T result = this->load(name);
        cached_items[name] = result;
        return result;
    }

protected:
    virtual T load(const string& name) = 0;
private:
    std::unordered_map<string, T> cached_items;
};

}//namespace io
}//namespace sp


#endif//SP2_IO_DIRECT_LOADER_H
