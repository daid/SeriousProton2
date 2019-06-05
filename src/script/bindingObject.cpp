#include <sp2/script/bindingObject.h>
#include <sp2/logging.h>
#include <lua/lua.hpp>
#include <typeinfo>

namespace sp {
namespace script {

int lazyLoadingIndex(lua_State* L)
{
    lazyLoading(1, L);
    //Return the actual field that was requested by lazyloading.
    lua_gettable(L, 1);
    return 1;
}

int lazyLoadingNewIndex(lua_State* L)
{
    lazyLoading(1, L);
    //Set the actual field that was requested by lazyloading.
    lua_settable(L, 1);
    return 1;
}

static int luaIndexProxy(lua_State* L)
{
    lua_getmetatable(L, 1);
    lua_getfield(L, -1, "proxy_table");
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    if (lua_istable(L, -1))
    {
        lua_getfield(L, -1, "get");
        lua_call(L, 0, 1);
    }
    return 1;
}

static int luaNewIndexProxy(lua_State* L)
{
    lua_getmetatable(L, 1);
    lua_getfield(L, -1, "proxy_table");
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    if (lua_isfunction(L, -1))
        return luaL_error(L, "Tried to assign to object function, which is not allowed");
    if (lua_isboolean(L, -1))
        return luaL_error(L, "Tried to assign to object valid value, which is not allowed");
    if (lua_istable(L, -1))
    {
        lua_getfield(L, -1, "set");
        lua_pushvalue(L, 3);
        lua_call(L, 1, 0);
        return 0;
    }
    lua_pop(L, 3);
    lua_rawset(L, 1);
    return 0;
}


void lazyLoading(int table_index, lua_State* L)
{
    //Get the object reference for this object.
    lua_getfield(L, table_index, "__ptr");
    sp::ScriptBindingObject* sbc = static_cast<sp::ScriptBindingObject*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    
    //Create a new table as meta table.
    lua_newtable(L);
    //Create a new table to store functions and properties for this object.
    lua_newtable(L);
    int function_table_index = lua_gettop(L);
    //Put a field "valid" in this metatable that is always true. (We clear the metatable on object destruction, causing valid to become "nil" and thus false)
    lua_pushboolean(L, true);
    lua_setfield(L, function_table_index, "valid");
    
    //Call the onRegisterScriptBindings which will register functions in the current __index table.
    ScriptBindingClass script_binding_class(L, table_index, function_table_index);
    sbc->onRegisterScriptBindings(script_binding_class);
    
    //Set the table as a field in the metatable so we can access it from our proxy function.
    lua_setfield(L, -2, "proxy_table");

    //Push our proxy functions for indexing and value assignment.
    lua_pushcfunction(L, luaIndexProxy);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, luaNewIndexProxy);
    lua_setfield(L, -2, "__newindex");
    
    //Register the metatable on our object table
    lua_setmetatable(L, table_index);
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
