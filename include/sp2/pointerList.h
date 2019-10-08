#ifndef SP2_POINTER_LIST_H
#define SP2_POINTER_LIST_H

#include <sp2/pointer.h>
#include <sp2/logging.h>

#include <vector>
#include <functional>
#include <algorithm>

namespace sp {

class _PListEntry;
class _PListIteratorBase;
class _PListReverseIteratorBase;

class _PListBase : NonCopyable
{
public:
    _PListBase()
    {
        list_start = nullptr;
        list_end = nullptr;
        iterators_start = nullptr;
        reverse_iterators_start = nullptr;
    }
    ~_PListBase();
protected:
    _PListEntry* list_start;
    _PListEntry* list_end;
    _PListIteratorBase* iterators_start;
    _PListReverseIteratorBase* reverse_iterators_start;

    void insert(AutoPointerObject* item);
    void preventIteratorsFromPointingAt(_PListEntry* e);
    _PListEntry* removeEntry(_PListEntry* e);
        
    friend class AutoPointerObject;
    friend class _PListIteratorBase;
    friend class _PListReverseIteratorBase;
};

class _PListEntry : NonCopyable
{
public:
    AutoPointerObject* object;
    _PListEntry* object_prev;
    _PListEntry* object_next;

    _PListBase* list;
    _PListEntry* list_prev;
    _PListEntry* list_next;

    void free();
    static _PListEntry* create();
};

class _PListIteratorBase : NonCopyable
{
public:
    _PListIteratorBase(_PListBase* list)
    : list(list)
    {
        if (list->list_start)
        {
            entry = list->list_start;
            iterator_next = list->iterators_start;
            iterator_prev = nullptr;
            list->iterators_start = this;
        }
        else
        {
            this->list = nullptr;
            this->entry = nullptr;
        }
    }
    
    _PListIteratorBase()
    {
        this->list = nullptr;
        this->entry = nullptr;
    }
    
    _PListIteratorBase(const _PListIteratorBase& other)
    : NonCopyable()
    {
        if (other.list)
        {
            list = other.list;
            entry = other.entry;
            iterator_next = list->iterators_start;
            iterator_prev = nullptr;
            list->iterators_start = this;
        }
        else
        {
            list = nullptr;
        }
    }
    
    ~_PListIteratorBase()
    {
        removeFromList();
    }

    bool operator==(const _PListIteratorBase& i) const
    {
        return list == i.list && entry == i.entry;
    }

    bool operator!=(const _PListIteratorBase& i) const
    {
        return list != i.list || entry != i.entry;
    }
    
    void operator++()
    {
        if (!entry && list)
            entry = list->list_start;
        else
            entry = entry->list_next;
        if (!entry)
        {
            removeFromList();
            list = nullptr;
        }
    }
    
    _PListIteratorBase& operator =(const _PListIteratorBase& other);
protected:
    _PListBase* list;
    _PListEntry* entry;
    _PListIteratorBase* iterator_next;
    _PListIteratorBase* iterator_prev;

    void removeFromList()
    {
        if (!list)
            return;
        if (iterator_next)
            iterator_next->iterator_prev = iterator_prev;
        if (iterator_prev)
            iterator_prev->iterator_next = iterator_next;
        else
            list->iterators_start = iterator_next;
    }
    
    friend class _PListBase;
};

class _PListReverseIteratorBase : NonCopyable
{
public:
    _PListReverseIteratorBase(_PListBase* list)
    : list(list)
    {
        if (list->list_end)
        {
            entry = list->list_end;
            iterator_next = list->reverse_iterators_start;
            iterator_prev = nullptr;
            list->reverse_iterators_start = this;
        }
        else
        {
            this->list = nullptr;
            this->entry = nullptr;
        }
    }
    
    _PListReverseIteratorBase()
    {
        this->list = nullptr;
        this->entry = nullptr;
    }
    
    _PListReverseIteratorBase(const _PListReverseIteratorBase& other)
    : NonCopyable()
    {
        if (other.list)
        {
            list = other.list;
            entry = other.entry;
            iterator_next = list->reverse_iterators_start;
            iterator_prev = nullptr;
            list->reverse_iterators_start = this;
        }
        else
        {
            list = nullptr;
        }
    }
    
    ~_PListReverseIteratorBase()
    {
        removeFromList();
    }

    bool operator==(const _PListReverseIteratorBase& i) const
    {
        return list == i.list && entry == i.entry;
    }

    bool operator!=(const _PListReverseIteratorBase& i) const
    {
        return list != i.list || entry != i.entry;
    }
    
    void operator++()
    {
        if (!entry && list)
            entry = list->list_end;
        else
            entry = entry->list_prev;
        if (!entry)
        {
            removeFromList();
            list = nullptr;
        }
    }
    
    _PListReverseIteratorBase& operator =(const _PListReverseIteratorBase& other);
protected:
    _PListBase* list;
    _PListEntry* entry;
    _PListReverseIteratorBase* iterator_next;
    _PListReverseIteratorBase* iterator_prev;

    void removeFromList()
    {
        if (!list)
            return;
        if (iterator_next)
            iterator_next->iterator_prev = iterator_prev;
        if (iterator_prev)
            iterator_prev->iterator_next = iterator_next;
        else
            list->reverse_iterators_start = iterator_next;
    }
    
    friend class _PListBase;
};

template<class T> class PList : _PListBase
{
public:
    class Iterator : public _PListIteratorBase
    {
    public:
        Iterator(_PListBase* list)
        : _PListIteratorBase(list)
        {
        }
        
        Iterator()
        : _PListIteratorBase()
        {
        }
    
        P<T> operator*()
        {
            return static_cast<T*>(entry->object);
        }
    };

    class ReverseIterator : public _PListReverseIteratorBase
    {
    public:
        ReverseIterator(_PListBase* list)
        : _PListReverseIteratorBase(list)
        {
        }
        
        ReverseIterator()
        : _PListReverseIteratorBase()
        {
        }
    
        P<T> operator*()
        {
            return static_cast<T*>(entry->object);
        }
    };

    ~PList()
    {
    }

    void add(P<T> item)
    {
        if (!item)
            return;
        
        insert(*item);
    }
    
    void remove(P<T> item)
    {
        if (item)
        {
            for(_PListEntry* e = list_start; e;)
            {
                if (e->object == *item)
                {
                    e = removeEntry(e);
                }
                else
                {
                    e = e->list_next;
                }
            }
        }
    }
    
    void clear()
    {
        _PListEntry* e = list_start;
        while(e)
            e = removeEntry(e);
    }

    Iterator begin() const
    {
        return Iterator(const_cast<PList*>(this));
    }

    const Iterator end() const
    {
        return Iterator();
    }

    ReverseIterator rbegin() const
    {
        return ReverseIterator(const_cast<PList*>(this));
    }
    
    const ReverseIterator rend() const
    {
        return ReverseIterator();
    }
    
    int size() const
    {
        int count = 0;
        for(_PListEntry* e = list_start; e; e=e->list_next)
            count++;
        return count;
    }

    bool empty() const
    {
        return list_start == nullptr;
    }

    void sort(std::function<int(const P<T>& a, const P<T>& b)> compare_function)
    {
        for(_PListEntry* e = list_start; e && e->list_next; e=e->list_next)
        {
            while(compare_function(static_cast<T*>(e->object), static_cast<T*>(e->list_next->object)) > 0)
            {
                //Swap e an e->list_next
                _PListEntry* prev = e->list_prev;
                _PListEntry* next = e->list_next;
                _PListEntry* next_next = e->list_next->list_next;
                
                if (prev)
                    prev->list_next = next;
                else
                    list_start = next;
                next->list_prev = prev;
                next->list_next = e;
                e->list_prev = next;
                e->list_next = next_next;
                if (next_next)
                    next_next->list_prev = e;
                else
                    list_end = e;
                
                //Set E to previous, so we try to swap our newly swapped in front furter forwards.
                // If there is no previous, we can just continue, but we do need to set e to "next" so we do not skip anything.
                if (prev)
                    e = prev;
                else
                    e = next;
            }
        }
    }
};

}//namespace sp

#endif//SP2_POINTER_VECTOR_H
