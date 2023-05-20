#include <sp2/io/lazyLoader.h>

namespace sp {
namespace io {

static bool running = true;
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
        atexit([](){
            running = false;
            queue.put([](){});
        });
        thread = new std::thread([]()
        {
            while(running)
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
