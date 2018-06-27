#include <sp2/script/bindingObject.h>
#include <sp2/logging.h>
#include <lua/lua.hpp>
#include <typeinfo>

namespace sp {
namespace script {

int lazyLoading(lua_State* L)
{
    //Het the object reference for this object.
    lua_getfield(L, 1, "__ptr");
    sp::ScriptBindingObject* sbc = static_cast<sp::ScriptBindingObject*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    
    //Create a new table as meta table.
    lua_newtable(L);
    lua_pushstring(L, "__index");
    //Create a new table as __index table for this object.
    lua_newtable(L);
    //Put a field "valid" in this metatable that is always true. (We clear the metatable on object destruction, causing valid to become "nil" and thus false)
    lua_pushboolean(L, true);
    lua_setfield(L, -2, "valid");
    
    //Call the onRegisterScriptBindings which will register functions in the current __index table.
    ScriptBindingClass script_binding_class(L);
    sbc->onRegisterScriptBindings(script_binding_class);
    
    //Set the __index table as actual field in the metatable.
    lua_settable(L, -3);
    //Register the metatable on our object table
    lua_setmetatable(L, 1);
    //Return the actual field that was requested by lazyloading.
    lua_gettable(L, 1);
    return 1;
}

};

ScriptBindingObject::ScriptBindingObject()
{
    if (!script::global_lua_state)
        script::createGlobalLuaState();

    //Add object to Lua registry, and register the lazy loader. This loads the bindings on first use, so we do not bind objects that we never use from the scripts.
    //REGISTY[this] = {"__ptr": this}
    lua_newtable(script::global_lua_state);
    lua_pushlightuserdata(script::global_lua_state, this);
    lua_setfield(script::global_lua_state, -2, "__ptr");
    luaL_setmetatable(script::global_lua_state, "lazyLoading");
    lua_rawsetp(script::global_lua_state, LUA_REGISTRYINDEX, this);
}

ScriptBindingObject::~ScriptBindingObject()
{
    //Clear our pointer reference in our object table
    //REGISTY[this]["__ptr"] = nullptr
    lua_rawgetp(script::global_lua_state, LUA_REGISTRYINDEX, this);
    lua_pushlightuserdata(script::global_lua_state, nullptr);
    lua_setfield(script::global_lua_state, -2, "__ptr");
    //Clear the metatable of this object.
    lua_pushnil(script::global_lua_state);
    lua_setmetatable(script::global_lua_state, -2);
    lua_pop(script::global_lua_state, 1);
    
    //Remove object from Lua registry
    //REGISTY[this] = nil
    lua_pushnil(script::global_lua_state);
    lua_rawsetp(script::global_lua_state, LUA_REGISTRYINDEX, this);
}

void ScriptBindingObject::setScriptMember(string name, int value)
{
    //REGISTY[this][name] = value
    lua_rawgetp(script::global_lua_state, LUA_REGISTRYINDEX, this);
    lua_pushinteger(script::global_lua_state, value);
    lua_setfield(script::global_lua_state, -2, name.c_str());
    lua_pop(script::global_lua_state, 1);
}

void ScriptBindingObject::setScriptMember(string name, double value)
{
    //REGISTY[this][name] = value
    lua_rawgetp(script::global_lua_state, LUA_REGISTRYINDEX, this);
    lua_pushnumber(script::global_lua_state, value);
    lua_setfield(script::global_lua_state, -2, name.c_str());
    lua_pop(script::global_lua_state, 1);
}

void ScriptBindingObject::setScriptMember(string name, string value)
{
    //REGISTY[this][name] = value
    lua_rawgetp(script::global_lua_state, LUA_REGISTRYINDEX, this);
    lua_pushstring(script::global_lua_state, value.c_str());
    lua_setfield(script::global_lua_state, -2, name.c_str());
    lua_pop(script::global_lua_state, 1);
}

int ScriptBindingObject::getScriptMemberInteger(string name)
{
    //return REGISTY[this][name]
    lua_rawgetp(script::global_lua_state, LUA_REGISTRYINDEX, this);
    lua_getfield(script::global_lua_state, -1, name.c_str());
    int result = lua_tointeger(script::global_lua_state, -1);
    lua_pop(script::global_lua_state, 2);
    return result;
}

double ScriptBindingObject::getScriptMemberDouble(string name)
{
    //return REGISTY[this][name]
    lua_rawgetp(script::global_lua_state, LUA_REGISTRYINDEX, this);
    lua_getfield(script::global_lua_state, -1, name.c_str());
    double result = lua_tonumber(script::global_lua_state, -1);
    lua_pop(script::global_lua_state, 2);
    return result;
}

string ScriptBindingObject::getScriptMemberString(string name)
{
    //return REGISTY[this][name]
    lua_rawgetp(script::global_lua_state, LUA_REGISTRYINDEX, this);
    lua_getfield(script::global_lua_state, -1, name.c_str());
    string result = lua_tostring(script::global_lua_state, -1);
    lua_pop(script::global_lua_state, 2);
    return result;
}

void ScriptBindingObject::onRegisterScriptBindings(ScriptBindingClass& script_binding_class)
{
}

};//namespace sp
