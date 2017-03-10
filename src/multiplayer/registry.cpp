#include <sp2/multiplayer/registry.h>

namespace sp {
namespace multiplayer {

ClassEntry* ClassEntry::list_start;
std::unordered_map<std::type_index, string> ClassEntry::type_to_name_mapping;
std::unordered_map<string, ClassEntry::create_object_function_t> ClassEntry::name_to_create_mapping;
   
ClassEntry::ClassEntry(string class_name, std::type_index type_index, create_object_function_t create_function)
: class_name(class_name), type_index(type_index), create_function(create_function)
{
    next = list_start;
    list_start = this;
}

void ClassEntry::fillMappings()
{
    for(ClassEntry* ce = list_start; ce; ce = ce->next)
    {
        type_to_name_mapping[ce->type_index] = ce->class_name;
        name_to_create_mapping[ce->class_name] = ce->create_function;
    }
}

};//!namespace multiplayer
};//!namespace sp
