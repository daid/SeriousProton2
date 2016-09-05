#include <sp2/pointer.h>

namespace sp {

AutoPointerObject::~AutoPointerObject()
{
    for(_PBase* p = pointer_list_start; p; p = p->next)
        p->ptr = nullptr;
}

};//!namespace sp
