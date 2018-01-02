#ifndef SP2_POINTER_VECTOR_H
#define SP2_POINTER_VECTOR_H

#include <sp2/pointer.h>

#include <vector>
#include <functional>
#include <algorithm>

namespace sp {

/** Depricated, use PList, which is safer and more efficient */
template<class T> class PVector
{
public:
    class Iterator
    {
    public:
        Iterator(PVector* v, int index)
        : v(v), index(index)
        {
            clearOutNullptr();
        }

        bool operator!=(const Iterator& i) const
        {
            return v != i.v || index != i.index;
        }
        
        void operator++()
        {
            index++;
            clearOutNullptr();
        }
        
        T* operator*()
        {
            return *v->items[index];
        }
    private:
        PVector* v;
        int index;
        
        void clearOutNullptr()
        {
            while(index != -1 && index < int(v->items.size()) && !v->items[index])
            {
                v->items.erase(v->items.begin() + index);
                if (index == int(v->items.size()))
                    index = -1;
            }
            if (index == int(v->items.size()))
                index = -1;
        }
    };
    class ReverseIterator
    {
    public:
        ReverseIterator(PVector* v, int index)
        : v(v), index(index)
        {
            clearOutNullptr();
        }
        
        bool operator!=(const ReverseIterator& i) const
        {
            return v != i.v || index != i.index;
        }
        
        void operator++()
        {
            index--;
            clearOutNullptr();
        }
        
        T* operator*()
        {
            return *v->items[index];
        }
    private:
        PVector* v;
        int index;
        
        void clearOutNullptr()
        {
            while(index != -1 && !v->items[index])
            {
                v->items.erase(v->items.begin() + index);
                index -= 1;
            }
        }
    };

    void add(P<T> item)
    {
        if (item)
            items.push_back(item);
    }
    
    void remove(P<T> item)
    {
        if (item)
        {
            for(auto it = items.begin(); it != items.end();)
            {
                if (*it == item)
                    it = items.erase(it);
                else
                    ++it;
            }
        }
    }

    Iterator begin()
    {
        return Iterator(this, 0);
    }

    const Iterator end()
    {
        return Iterator(this, -1);
    }
    
    ReverseIterator rbegin()
    {
        return ReverseIterator(this, items.size() - 1);
    }
    
    const ReverseIterator rend()
    {
        return ReverseIterator(this, -1);
    }
    
    int size()
    {
        return items.size();
    }
    
    void sort(std::function<int(const P<T>& a, const P<T>& b)> compare_function)
    {
        std::sort(items.begin(), items.end(), compare_function);
    }
private:
    void cleanOutNullptrs()
    {
        items.erase(std::remove_if(items.begin(), items.end(), [](const P<T>& p) { return !bool(p); }), items.end());
    }

    std::vector<P<T>> items;
};

};//!namespace sp

#endif//SP2_POINTER_VECTOR_H
