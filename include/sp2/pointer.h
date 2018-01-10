#ifndef SP2_POINTER_H
#define SP2_POINTER_H

#include <SFML/System/NonCopyable.hpp>
#include <sp2/pointerBase.h>
#include <ostream>
#include <typeinfo>

namespace sp {

class AutoPointerObject : sf::NonCopyable
{
private:
    _PBase* pointer_list_start;
    _PListEntry* pointer_list_entry_list_start;
public:
    AutoPointerObject()
    {
        pointer_list_start = nullptr;
        pointer_list_entry_list_start = nullptr;
    }
    
    virtual ~AutoPointerObject();
    
    friend class _PBase;
    friend class _PListBase;
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
    P(P&& p)
    {
        take(p);
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

    explicit operator bool() const
    {
        return ptr != nullptr;
    }

    void destroy()
    {
        if (ptr)
            delete ptr;
    }
    
    bool operator==(const P<T>& other)
    {
        return ptr == other.ptr;
    }

    bool operator!=(const P<T>& other)
    {
        return ptr != other.ptr;
    }

    template<class T2> operator P<T2>()
    {
        return dynamic_cast<T2*>(**this);
    }
};

template<typename T> static inline std::ostream& operator<<(std::ostream& os, const P<T>& p)
{
    if (bool(p))
        os << typeid(**p).name() << ':' << *p;
    else
        os << "nullptr";
    return os;
}

};//!namespace sp

#endif//SP2_POINTER_H
