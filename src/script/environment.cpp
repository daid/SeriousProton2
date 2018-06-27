#include <sp2/script/environment.h>
#include <sp2/script/luaBindings.h>

namespace sp {
namespace script {

Environment::Environment()
{
    if (!script::global_lua_state)
        script::createGlobalLuaState();

    //Create a new lua environment.
    //REGISTY[this] = {"metatable": {"__index": _G}, "__ptr": this}    
    lua_newtable(global_lua_state); //environment
    
    lua_newtable(global_lua_state); //environment metatable
    lua_pushglobaltable(global_lua_state);
    lua_setfield(global_lua_state, -2, "__index");
    lua_setmetatable(global_lua_state, -2);
    
    //Create __ptr in this environment.
    lua_pushlightuserdata(global_lua_state, this);
    lua_setfield(global_lua_state, -2, "__ptr");
    
    lua_rawsetp(global_lua_state, LUA_REGISTRYINDEX, this);
}

Environment::~Environment()
{
    //Remove our environment from the registry.
    //REGISTRY[this] = nil
    lua_pushnil(global_lua_state);
    lua_rawsetp(global_lua_state, LUA_REGISTRYINDEX, this);
}

void Environment::setGlobal(string name, lua_CFunction function)
{
    //Get the environment table from the registry.
    lua_rawgetp(global_lua_state, LUA_REGISTRYINDEX, this);
    
    //Set our variable in this environment table, with our environment as first upvalue.
    if (function)
    {
        lua_pushvalue(global_lua_state, -1);
        lua_pushcclosure(global_lua_state, function, 1);
    }
    else
    {
        lua_pushnil(global_lua_state);
    }
    lua_setfield(global_lua_state, -2, name.c_str());
    
    //Pop the table
    lua_pop(global_lua_state, 1);
}

void Environment::setGlobal(string name, P<ScriptBindingObject> ptr)
{
    //Get the environment table from the registry.
    lua_rawgetp(global_lua_state, LUA_REGISTRYINDEX, this);
    
    //Set our variable in this environment table
    pushToLua(global_lua_state, ptr);
    lua_setfield(global_lua_state, -2, name.c_str());
    
    //Pop the table
    lua_pop(global_lua_state, 1);
}

bool Environment::load(sp::io::ResourceStreamPtr resource)
{
    if (!resource)
        return false;

    string filecontents = resource->readAll();

    if (luaL_loadbuffer(global_lua_state, filecontents.c_str(), filecontents.length(), nullptr))
    {
        string error_string = luaL_checkstring(global_lua_state, -1);
        LOG(Error, "LUA: load:", error_string);
        lua_pop(global_lua_state, 1);
        return false;
    }

    //Get the environment table from the registry.
    lua_rawgetp(global_lua_state, LUA_REGISTRYINDEX, this);
    //set the environment table it as 1st upvalue
    lua_setupvalue(global_lua_state, -2, 1);
    
    //Call the actual code.
    if (lua_pcall(global_lua_state, 0, 0, 0))
    {
        string error_string = luaL_checkstring(global_lua_state, -1);
        LOG(Error, "LUA: run:", error_string);
        lua_pop(global_lua_state, 1);
        return false;
    }
    return true;
}

};//namespace script
};//namespace sp
