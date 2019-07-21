#include <sp2/script/environment.h>
#include <sp2/script/luaBindings.h>

namespace sp {
namespace script {

Environment::Environment()
{
    if (!script::global_lua_state)
        script::global_lua_state = script::createLuaState();
    lua = script::global_lua_state;

    //Create a new lua environment.
    //REGISTY[this] = {"metatable": {"__index": _G, "environment_ptr": this}}
    lua_newtable(lua); //environment
    
    lua_newtable(lua); //environment metatable
    lua_pushstring(lua, "[environment]");
    lua_setfield(lua, -2, "__metatable");
    lua_pushglobaltable(lua);
    lua_setfield(lua, -2, "__index");
    //Set the ptr in the metatable.
    lua_pushlightuserdata(lua, this);
    lua_setfield(lua, -2, "environment_ptr");

    lua_setmetatable(lua, -2);
    
    lua_rawsetp(lua, LUA_REGISTRYINDEX, this);
}

Environment::~Environment()
{
    //Remove our environment from the registry.
    //REGISTRY[this] = nil
    lua_pushnil(lua);
    lua_rawsetp(lua, LUA_REGISTRYINDEX, this);
}

void Environment::setGlobal(string name, lua_CFunction function)
{
    //Get the environment table from the registry.
    lua_rawgetp(lua, LUA_REGISTRYINDEX, this);
    
    //Set our variable in this environment table, with our environment as first upvalue.
    if (function)
    {
        lua_pushvalue(lua, -1);
        lua_pushcclosure(lua, function, 1);
    }
    else
    {
        lua_pushnil(lua);
    }
    lua_setfield(lua, -2, name.c_str());
    
    //Pop the table
    lua_pop(lua, 1);
}

void Environment::setGlobal(string name, ScriptBindingObject* ptr)
{
    //Get the environment table from the registry.
    lua_rawgetp(lua, LUA_REGISTRYINDEX, this);
    
    //Set our variable in this environment table
    pushToLua(lua, ptr);
    lua_setfield(lua, -2, name.c_str());
    
    //Pop the table
    lua_pop(lua, 1);
}

void Environment::setGlobal(string name, P<ScriptBindingObject> ptr)
{
    setGlobal(name, *ptr);
}

void Environment::setGlobal(string name, bool value)
{
    //Get the environment table from the registry.
    lua_rawgetp(lua, LUA_REGISTRYINDEX, this);
    
    //Set our variable in this environment table
    lua_pushboolean(lua, value);
    lua_setfield(lua, -2, name.c_str());
    
    //Pop the table
    lua_pop(lua, 1);
}

void Environment::setGlobal(string name, int value)
{
    //Get the environment table from the registry.
    lua_rawgetp(lua, LUA_REGISTRYINDEX, this);
    
    //Set our variable in this environment table
    lua_pushinteger(lua, value);
    lua_setfield(lua, -2, name.c_str());
    
    //Pop the table
    lua_pop(lua, 1);
}

void Environment::setGlobal(string name, string value)
{
    //Get the environment table from the registry.
    lua_rawgetp(lua, LUA_REGISTRYINDEX, this);
    
    //Set our variable in this environment table
    lua_pushstring(lua, value.c_str());
    lua_setfield(lua, -2, name.c_str());
    
    //Pop the table
    lua_pop(lua, 1);
}

bool Environment::load(string resource_name)
{
    io::ResourceStreamPtr stream = io::ResourceProvider::get(resource_name);
    if (!stream)
    {
        LOG(Warning, "Failed to find script resource:", resource_name);
        return false;
    }
    return _load(stream, resource_name);
}

bool Environment::load(io::ResourceStreamPtr resource)
{
    return _load(resource, "[unknown]");
}

bool Environment::_load(io::ResourceStreamPtr resource, string name)
{
    if (!resource)
        return false;

    string filecontents = resource->readAll();

    if (luaL_loadbufferx(lua, filecontents.c_str(), filecontents.length(), name.c_str(), "t"))
    {
        last_error = string("LUA: load: ") + luaL_checkstring(lua, -1);
        LOG(Error, last_error);
        lua_pop(lua, 1);
        return false;
    }

    //Get the environment table from the registry.
    lua_rawgetp(lua, LUA_REGISTRYINDEX, this);
    //set the environment table it as 1st upvalue
    lua_setupvalue(lua, -2, 1);
    
    //Call the actual code.
    if (lua_pcall(lua, 0, 0, 0))
    {
        last_error = string("LUA: run: ") + luaL_checkstring(lua, -1);
        LOG(Error, last_error);
        lua_pop(lua, 1);
        return false;
    }
    return true;
}

};//namespace script
};//namespace sp
