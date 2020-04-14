#include <sp2/io/lazyLoader.h>

namespace sp {
namespace io {

std::thread* LazyLoaderManager::thread;
sp::threading::Queue<std::function<void()>> LazyLoaderManager::queue;

void LazyLoaderManager::addWork(std::function<void()> f)
{
#ifdef __EMSCRIPTEN__
    //emscripten has very limited threading support, so do not lazy load with emscripten.
    f();
#else
    queue.put(f);

    if (!thread)
    {
        thread = new std::thread([]()
        {
            while(true)
            {
                std::function<void()> workFunction = queue.get();
                workFunction();
            }
        });
    }
#endif
}

}//namespace io
}//namespace sp
