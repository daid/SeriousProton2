#ifndef SP2_MULTIPLAYER_REGISTRY_H
#define SP2_MULTIPLAYER_REGISTRY_H

#include <sp2/string.h>
#include <sp2/pointer.h>

#include <typeindex>
#include <unordered_map>

namespace sp {
class Engine;
class Node;
namespace multiplayer {

//Register a class name to a string for multiplayer replication.
//TOFIX: This does not work very well in combination with namespaces.
#define REGISTER_MULTIPLAYER_CLASS(class_name) \
    ::sp::multiplayer::ClassEntry class_entry_ ## class_name ( # class_name , typeid(class_name), ::sp::multiplayer::__objectCreateFunction<class_name> );

class ClassEntry
{
public:
    typedef Node* (*create_object_function_t)(P<Node> parent);
    
    ClassEntry(string class_name, std::type_index type_index, create_object_function_t create_function);
private:
    string class_name;
    std::type_index type_index;
    create_object_function_t create_function;

    ClassEntry* next;
    static ClassEntry* list_start;
    
    static void fillMappings();
    
    static std::unordered_map<std::type_index, string> type_to_name_mapping;
    static std::unordered_map<string, create_object_function_t> name_to_create_mapping;
    
    friend class ::sp::Engine;
    friend class Server;
    friend class Client;
};

template<class T, class = typename std::enable_if<std::is_base_of<Node, T>::value>::type> Node* __objectCreateFunction(P<Node> parent)
{
    return new T(parent);
}

};//namespace multiplayer
};//namespace sp

#endif//SP2_MULTIPLAYER_CLIENT_H
