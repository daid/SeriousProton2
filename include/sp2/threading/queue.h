#ifndef SP2_THREADING_QUEUE_H
#define SP2_THREADING_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

namespace sp {
namespace threading {

template<class T> class Queue
{
public:
    void put(T item)
    {
        std::lock_guard<std::mutex> lock(mutex);
        queue.push(item);
        condition.notify_one();
    }
    
    T get()
    {
        std::unique_lock<std::mutex> lock(mutex);
        while(queue.empty())
            condition.wait(lock);
        T result = queue.front();
        queue.pop();
        return result;
    }
private:
    std::queue<T> queue;
    std::mutex mutex;
    std::condition_variable condition;
};

};//!namespace threading
};//!namespace sp


#endif//SP2_THREADING_QUEUE_H
