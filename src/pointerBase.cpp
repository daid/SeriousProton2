#include <sp2/pointerBase.h>
#include <sp2/pointer.h>

namespace sp {

void _PBase::release()
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

void _PBase::set(AutoPointerObject* p)
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

void _PBase::take(_PBase& p)
{
    ptr = p.ptr;
    next = p.next;
    prev = p.prev;
    
    if (ptr)
    {
        if (prev)
            prev->next = this;
        else
            ptr->pointer_list_start = this;
        if (next)
            next->prev = this;
    }
    p.ptr = nullptr;
}

}//!namespace sp
