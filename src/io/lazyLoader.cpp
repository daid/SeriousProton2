#include <sp2/io/lazyLoader.h>

namespace sp {
namespace io {

std::thread* LazyLoaderManager::thread;
sp::threading::Queue<std::function<void()>> LazyLoaderManager::queue;

void LazyLoaderManager::addWork(std::function<void()> f)
{
    queue.put(f);

    if (!thread)
    {
        thread = new std::thread([]()
        {
            while(true)
            {
                std::function<void()> f = queue.get();
                f();
            }
        });
    }
}

};//namespace io
};//namespace sp
