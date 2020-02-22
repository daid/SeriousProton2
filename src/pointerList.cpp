#include <sp2/pointerList.h>

namespace sp {

static _PListEntry* free_list;

_PListBase::~_PListBase()
{
    _PListEntry* e = list_start;
    while(e)
        e = removeEntry(e);

    for(_PListIteratorBase* i = iterators_start; i; i=i->iterator_next)
    {
        i->entry = nullptr;
        i->list = nullptr;
    }
    for(_PListReverseIteratorBase* i = reverse_iterators_start; i; i=i->iterator_next)
    {
        i->entry = nullptr;
        i->list = nullptr;
    }
}

void _PListBase::preventIteratorsFromPointingAt(_PListEntry* e)
{
    for(_PListIteratorBase* i = iterators_start; i; i=i->iterator_next)
    {
        if (i->entry == e)
        {
            i->entry = i->entry->list_prev;
        }
    }
    for(_PListReverseIteratorBase* i = reverse_iterators_start; i; i=i->iterator_next)
    {
        if (i->entry == e)
        {
            i->entry = i->entry->list_next;
        }
    }
}

_PListEntry* _PListBase::removeEntry(_PListEntry* e)
{
    _PListEntry* next = e->object_next;
    if (next)
        next->object_prev = e->object_prev;
    if (e->object_prev)
        e->object_prev->object_next = next;
    else
        e->object->pointer_list_entry_list_start = next;

    next = e->list_next;
    if (next)
        next->list_prev = e->list_prev;
    else
        list_end = e->list_prev;
    if (e->list_prev)
        e->list_prev->list_next = next;
    else
        list_start = next;
    e->free();
    return next;
}

void _PListEntry::free()
{
    list_next = free_list;
    free_list = this;
}

_PListEntry* _PListEntry::create()
{
    _PListEntry* result = free_list;
    if (result)
    {
        free_list = free_list->list_next;
    }
    else
    {
        result = new _PListEntry();
    }
    return result;
}

void _PListBase::insert(AutoPointerObject* item)
{
    _PListEntry* new_entry = _PListEntry::create();
    new_entry->object = item;
    new_entry->object_prev = nullptr;
    new_entry->object_next = new_entry->object->pointer_list_entry_list_start;
    new_entry->object->pointer_list_entry_list_start = new_entry;
    
    new_entry->list = this;
    new_entry->list_next = nullptr;
    new_entry->list_prev = list_end;
    if (list_end)
        list_end->list_next = new_entry;
    else
        list_start = new_entry;
    list_end = new_entry;
}

}//namespace sp
