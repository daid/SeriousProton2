#include <sp2/pointer.h>
#include <sp2/pointerList.h>

namespace sp {

AutoPointerObject::~AutoPointerObject()
{
    for(_PBase* p = pointer_list_start; p; p = p->next)
        p->ptr = nullptr;
    
    for(_PListEntry* e = pointer_list_entry_list_start; e;)
    {
        _PListEntry* next = e->object_next;
        
        e->list->preventIteratorsFromPointingAt(e);
        
        if (e->list_prev)
            e->list_prev->list_next = e->list_next;
        else
            e->list->list_start = e->list_next;
        if (e->list_next)
            e->list_next->list_prev = e->list_prev;
        else
            e->list->list_end = e->list_prev;
        
        e->free();
        e = next;
    }
}

};//!namespace sp
