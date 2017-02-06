#include <sp2/script/bindingObject.h>
#include <sp2/logging.h>
#include <lua/lua.hpp>
#include <typeinfo>

namespace sp {
namespace script {

int lazyLoading(lua_State* L)
{
    lua_pushstring(L, "__ptr");
    lua_rawget(L, 1);
    sp::ScriptBindingObject* sbc = static_cast<sp::ScriptBindingObject*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    lua_newtable(L);
    lua_pushstring(L, "__index");
    lua_newtable(L);
    ScriptBindingClass script_binding_class;
    sbc->onRegisterScriptBindings(script_binding_class);
    lua_settable(L, -3);
    lua_setmetatable(L, 1);
    lua_gettable(L, 1);
    return 1;
}

};

ScriptBindingObject::ScriptBindingObject()
{
    if (!script::global_lua_state)
    {
        script::global_lua_state = luaL_newstate();
        luaL_newmetatable(script::global_lua_state, "lazyLoading");
        lua_pushstring(script::global_lua_state, "__index");
        lua_pushcfunction(script::global_lua_state, script::lazyLoading);
        lua_settable(script::global_lua_state, -3);
        lua_pop(script::global_lua_state, 1);
    }

    //Add object to Lua registry
    //REGISTY[this] = {"__ptr": this}
    lua_pushlightuserdata(script::global_lua_state, this);
    lua_newtable(script::global_lua_state);
    lua_pushstring(script::global_lua_state, "__ptr");
    lua_pushlightuserdata(script::global_lua_state, this);
    lua_rawset(script::global_lua_state, -3);
    luaL_setmetatable(script::global_lua_state, "lazyLoading");
    lua_settable(script::global_lua_state, LUA_REGISTRYINDEX);
}

ScriptBindingObject::~ScriptBindingObject()
{
    //Clear our pointer reference in our object table
    //REGISTY[this]["__ptr"] = nullptr
    lua_pushlightuserdata(script::global_lua_state, this);
    lua_gettable(script::global_lua_state, LUA_REGISTRYINDEX);
    lua_pushstring(script::global_lua_state, "__ptr");
    lua_pushlightuserdata(script::global_lua_state, nullptr);
    lua_rawset(script::global_lua_state, -3);
    lua_pop(script::global_lua_state, 1);
    
    //Remove object from Lua registry
    //REGISTY[this] = nil
    lua_pushlightuserdata(script::global_lua_state, this);
    lua_pushnil(script::global_lua_state);
    lua_settable(script::global_lua_state, LUA_REGISTRYINDEX);
}

void ScriptBindingObject::onRegisterScriptBindings(ScriptBindingClass& script_binding_class)
{
    LOG(Debug, "onRegisterScriptBindings:", typeid(*this).name());
}

};//!namespace sp
