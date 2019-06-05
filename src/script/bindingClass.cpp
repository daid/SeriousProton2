#include <sp2/script/bindingClass.h>
#include <sp2/script/callback.h>

namespace sp {

static int updateCallback(lua_State* L)
{
    if (!lua_isnil(L, 1))
        luaL_checktype(L, 1, LUA_TFUNCTION);
    
    lua_pushstring(L, "__ptr");
    lua_gettable(L, lua_upvalueindex(2));
    void* obj = lua_touserdata(L, -1);
    lua_pop(L, 1);
    if (!obj)   //Object was destroyed.
        return 0;
    
    //REGISTRY[callback_ptr] = parameter(1)
    lua_pushvalue(L, lua_upvalueindex(1));//The pointer of this callback.
    lua_pushvalue(L, 1);
    lua_settable(L, LUA_REGISTRYINDEX);
    return 0;
}

void ScriptBindingClass::bind(sp::string name, sp::script::Callback& callback)
{
    lua_pushlightuserdata(L, &callback);
    lua_pushvalue(L, object_table_index); //push the table of this object
    
    lua_pushcclosure(L, updateCallback, 2);
    lua_setfield(L, function_table_index, name.c_str());
}

};//namespace sp
