#ifndef SP2_POINTER_H
#define SP2_POINTER_H

#include <SFML/System/NonCopyable.hpp>

namespace sp {

class _PBase;

class AutoPointerObject : sf::NonCopyable
{
private:
    _PBase* pointer_list_start;

public:
    AutoPointerObject()
    {
        pointer_list_start = nullptr;
    }
    
    virtual ~AutoPointerObject();
    
    friend class _PBase;
};

class _PBase
{
protected:
    AutoPointerObject* ptr;
    _PBase* next;
    _PBase* prev;
    
    _PBase()
    {
        ptr = nullptr;
    }

    void release()
    {
        if (ptr)
        {
            if (prev)
            {
                prev->next = next;
                if (next)
                    next->prev = prev;
            }else{
                ptr->pointer_list_start = next;
                if (next)
                    next->prev = nullptr;
            }
        }
    }
    
    void set(AutoPointerObject* p)
    {
        release();
        ptr = p;
        if (ptr != nullptr)
        {
            next = ptr->pointer_list_start;
            ptr->pointer_list_start = this;
            prev = nullptr;
            if (next)
                next->prev = this;
        }
    }

    friend class AutoPointerObject;
};

template<class T> class P : _PBase
{
public:
    P()
    {
    }
    
    P(const P& p)
    {
        set(p.ptr);
    }
    P(T* p)
    {
        set(p);
    }

    ~P()
    {
        release();
    }

    P& operator = (T* p)
    {
        set(p);
        return *this;
    }

    P& operator = (const P& p)
    {
        if (&p != this) set(p.ptr);
        return *this;
    }

    T* operator->() const
    {
        return static_cast<T*>(ptr);
    }
    T* operator*() const
    {
        return static_cast<T*>(ptr);
    }

    explicit operator bool()
    {
        return ptr != nullptr;
    }

    void destroy()
    {
        if (ptr)
            delete ptr;
    }

    template<class T2> operator P<T2>()
    {
        return dynamic_cast<T2*>(**this);
    }
};

};//!namespace sp

#endif//SP2_POINTER_H
