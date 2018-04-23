#ifndef SP2_POINTER_BASE_H
#define SP2_POINTER_BASE_H

#include <sp2/nonCopyable.h>

namespace sp {

class _PBase;
class _PListEntry;
class AutoPointerObject;

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

    void release();
    
    void set(AutoPointerObject* p);
    void take(_PBase& p);

    friend class AutoPointerObject;
};

};//namespace sp

#endif//SP2_POINTER_H
