#ifndef SP2_POINTER_H
#define SP2_POINTER_H

#include <sp2/nonCopyable.h>
#include <sp2/pointerBase.h>
#include <ostream>
#include <typeinfo>

namespace sp {

/** Base class for objects that can be used in \ref sp::P<T>

    Any AutoPointerObject that is deleted will set all the \ref sp::P<T> that point to this object to nullptr.
    This means a AutoPointerObject can be safely deleted without leaving any dangling pointers.
    
    Most SeriousProton2 objects are a subclass AutoPointerObject.
 */
class AutoPointerObject : NonCopyable
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

/** Smart pointer with reference clearing on deletion.
    
    \ref sp::P<T> objects act like a pointer to T, where T must be a subclass of AutoPointerObject.
    The big difference between a normal pointer an an \ref sp::P<T> pointer is that the \ref sp::P<T> pointer will be set to nullptr as soon as the referenced object is deleted.
    This allows for safe deletion of objects without any dangling pointers.
    
    As it is common for objects in a game to be deleted while still having some reference somewhere.
    For general use, a \ref sp::P<T> object acts the same as a T* pointer.
    
    Example:
    \code
    class Obj : public sp::AutoPointerObject {...}
    
    //Creation of objects uses unmodified new.
    sp::P<Obj> obj = new Obj;
    sp::P<Obj> obj2 = obj;
    
    //We can access members and functions, just as if it was a normal pointer.
    obj->member = 10;
    if (obj) obj->function();
    
    obj2.destroy();  //Delete the object.
    
    if (obj) LOG(Error, "This will never happen");
    
    obj2 = nullptr;
    \endcode
 */
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

    explicit operator bool() const
    {
        return ptr != nullptr;
    }

    /** Delete the object to which this \ref sp::P<T> is pointing.
        
        Deletes the object that is being point to, and thus sets all references to it to nullptr.
        This function can be used even if the pointer is pointing to nullptr.
     */
    void destroy()
    {
        if (ptr)
            delete ptr;
    }

    bool operator==(const P<T>& other)
    {
        return ptr == other.ptr;
    }

    bool operator==(const T* other)
    {
        return ptr == other;
    }

    bool operator!=(const P<T>& other)
    {
        return ptr != other.ptr;
    }

    bool operator!=(const T* other)
    {
        return ptr != other;
    }

    template<class T2> operator P<T2>()
    {
        return dynamic_cast<T2*>(**this);
    }
};

template<typename T, class = typename std::enable_if<std::is_base_of<AutoPointerObject, T>::value>::type> static inline std::ostream& operator<<(std::ostream& os, const P<T>& p)
{
    if (bool(p))
        os << typeid(**p).name() << ':' << *p;
    else
        os << "nullptr";
    return os;
}

}//namespace sp

#endif//SP2_POINTER_H
