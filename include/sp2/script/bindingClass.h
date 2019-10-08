#ifndef SP2_SCRIPT_BINDING_CLASS_H
#define SP2_SCRIPT_BINDING_CLASS_H

#include <lua/lua.hpp>
#include <sp2/pointer.h>
#include <sp2/string.h>
#include <sp2/script/luaBindings.h>
#include <sp2/logging.h>

namespace sp {
namespace script { class Callback; };
class ScriptBindingObject;

class ScriptBindingClass
{
public:
    template<class TYPE, typename RET, typename... ARGS> void bind(sp::string name, RET(TYPE::*func)(ARGS...))
    {
        typedef RET(TYPE::*FT)(ARGS...);
        
        FT* f = reinterpret_cast<FT*>(lua_newuserdata(L, sizeof(FT)));
        *f = func;
        lua_pushvalue(L, object_table_index); //push the table of this object
        
        lua_pushcclosure(L, &script::callMember<TYPE, RET, ARGS...>, 2);
        lua_setfield(L, function_table_index, name.c_str());
    }

    void bind(sp::string name, sp::script::Callback& callback);
    
    template<class TYPE> void bindProperty(const sp::string& name, TYPE& t)
    {
        lua_newtable(L);
        lua_pushlightuserdata(L, &t);
        lua_pushcclosure(L, &script::getProperty<TYPE>, 1);
        lua_setfield(L, -2, "get");
        lua_pushlightuserdata(L, &t);
        lua_pushcclosure(L, &script::setProperty<TYPE>, 1);
        lua_setfield(L, -2, "set");
        lua_setfield(L, function_table_index, name.c_str());
    }

    template<class OBJECT_TYPE, typename PROPERTY_TYPE> void bindProperty(const sp::string& name, PROPERTY_TYPE(OBJECT_TYPE::*getter)(), void(OBJECT_TYPE::*setter)(PROPERTY_TYPE))
    {
        typedef PROPERTY_TYPE(OBJECT_TYPE::*GET_FT)();
        typedef void(OBJECT_TYPE::*SET_FT)(PROPERTY_TYPE);
        
        lua_newtable(L);

        GET_FT* get_ptr = reinterpret_cast<GET_FT*>(lua_newuserdata(L, sizeof(GET_FT)));
        *get_ptr = getter;
        lua_pushvalue(L, object_table_index); //push the table of this object
        lua_pushcclosure(L, &script::callMember<OBJECT_TYPE, PROPERTY_TYPE>, 2);
        lua_setfield(L, -2, "get");
        
        SET_FT* set_ptr = reinterpret_cast<SET_FT*>(lua_newuserdata(L, sizeof(SET_FT)));
        *set_ptr = setter;
        lua_pushvalue(L, object_table_index); //push the table of this object
        lua_pushcclosure(L, &script::callMember<OBJECT_TYPE, void, PROPERTY_TYPE>, 2);
        lua_setfield(L, -2, "set");

        lua_setfield(L, function_table_index, name.c_str());
    }
private:
    ScriptBindingClass(lua_State* L, int object_table_index, int function_table_index)
    : L(L), object_table_index(object_table_index), function_table_index(function_table_index) {}
    
    lua_State* L;
    int object_table_index;
    int function_table_index;
    
    friend void script::lazyLoading(int table_index, lua_State* L);
};

}//namespace sp

#include <sp2/script/callback.h>

#endif//SP2_SCRIPT_BINDING_OBJECT_H
