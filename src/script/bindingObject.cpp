#include <sp2/script/bindingObject.h>
#include <sp2/logging.h>
#include <lua/lua.hpp>
#include <typeinfo>

namespace sp {
namespace script {

int lazyLoading(lua_State* L)
{
    //Het the object reference for this object.
    lua_pushstring(L, "__ptr");
    lua_rawget(L, 1);
    sp::ScriptBindingObject* sbc = static_cast<sp::ScriptBindingObject*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    
    //Create a new table as meta table.
    lua_newtable(L);
    lua_pushstring(L, "__index");
    //Create a new table as __index table for this object.
    lua_newtable(L);
    //Put a field "valid" in this metatable that is always true. (We clear the metatable on object destruction, causing valid to become "nil" and thus false)
    lua_pushstring(L, "valid");
    lua_pushboolean(L, true);
    lua_settable(L, -3);
    
    //Call the onRegisterScriptBindings which will register functions in the current __index table.
    ScriptBindingClass script_binding_class;
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
    //Clear the metatable of this object.
    lua_pushnil(script::global_lua_state);
    lua_setmetatable(script::global_lua_state, -2);
    lua_pop(script::global_lua_state, 1);
    
    //Remove object from Lua registry
    //REGISTY[this] = nil
    lua_pushlightuserdata(script::global_lua_state, this);
    lua_pushnil(script::global_lua_state);
    lua_settable(script::global_lua_state, LUA_REGISTRYINDEX);
}

void ScriptBindingObject::setScriptMember(string name, int value)
{
    //REGISTY[this][name] = value
    lua_pushlightuserdata(script::global_lua_state, this);
    lua_gettable(script::global_lua_state, LUA_REGISTRYINDEX);
    lua_pushstring(script::global_lua_state, name.c_str());
    lua_pushinteger(script::global_lua_state, value);
    lua_rawset(script::global_lua_state, -3);
    lua_pop(script::global_lua_state, 1);
}

void ScriptBindingObject::setScriptMember(string name, double value)
{
    //REGISTY[this][name] = value
    lua_pushlightuserdata(script::global_lua_state, this);
    lua_gettable(script::global_lua_state, LUA_REGISTRYINDEX);
    lua_pushstring(script::global_lua_state, name.c_str());
    lua_pushnumber(script::global_lua_state, value);
    lua_rawset(script::global_lua_state, -3);
    lua_pop(script::global_lua_state, 1);
}

void ScriptBindingObject::setScriptMember(string name, string value)
{
    //REGISTY[this][name] = value
    lua_pushlightuserdata(script::global_lua_state, this);
    lua_gettable(script::global_lua_state, LUA_REGISTRYINDEX);
    lua_pushstring(script::global_lua_state, name.c_str());
    lua_pushstring(script::global_lua_state, value.c_str());
    lua_rawset(script::global_lua_state, -3);
    lua_pop(script::global_lua_state, 1);
}

int ScriptBindingObject::getScriptMemberInteger(string name)
{
    //return REGISTY[this][name]
    lua_pushlightuserdata(script::global_lua_state, this);
    lua_gettable(script::global_lua_state, LUA_REGISTRYINDEX);
    lua_pushstring(script::global_lua_state, name.c_str());
    lua_rawget(script::global_lua_state, -2);
    int result = lua_tointeger(script::global_lua_state, -1);
    lua_pop(script::global_lua_state, 2);
    return result;
}

double ScriptBindingObject::getScriptMemberDouble(string name)
{
    //return REGISTY[this][name]
    lua_pushlightuserdata(script::global_lua_state, this);
    lua_gettable(script::global_lua_state, LUA_REGISTRYINDEX);
    lua_pushstring(script::global_lua_state, name.c_str());
    lua_rawget(script::global_lua_state, -2);
    double result = lua_tonumber(script::global_lua_state, -1);
    lua_pop(script::global_lua_state, 2);
    return result;
}

string ScriptBindingObject::getScriptMemberString(string name)
{
    //return REGISTY[this][name]
    lua_pushlightuserdata(script::global_lua_state, this);
    lua_gettable(script::global_lua_state, LUA_REGISTRYINDEX);
    lua_pushstring(script::global_lua_state, name.c_str());
    lua_rawget(script::global_lua_state, -2);
    string result = lua_tostring(script::global_lua_state, -1);
    lua_pop(script::global_lua_state, 2);
    return result;
}

void ScriptBindingObject::onRegisterScriptBindings(ScriptBindingClass& script_binding_class)
{
}

};//!namespace sp
