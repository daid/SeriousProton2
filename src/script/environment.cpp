#include <sp2/script/environment.h>
#include <sp2/script/luaBindings.h>
#include <sp2/assert.h>
#include <lua/lstate.h>

static void luaInstructionCountHook(lua_State *L, lua_Debug *ar)
{
    luaL_error(L, "Instruction count exceeded.");
}

static void* luaAlloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
    sp::script::Environment::AllocInfo* info = static_cast<sp::script::Environment::AllocInfo*>(ud);
    if (ptr)
        info->total -= osize;
    if (info->in_protected_call && osize < nsize && info->total + nsize > info->max)
    {
        info->total += osize;
        return nullptr;
    }
    info->total += nsize;
    if (nsize == 0)
    {
        free(ptr);
        return nullptr;
    }
    return realloc(ptr, nsize);
}

namespace sp {
namespace script {

Environment::Environment()
{
    lua = createLuaState(this);

    //In case a environment is created from within a lua function this assert fails.
    //sp2assert(lua_gettop(lua) == 0, "Lua stack incorrect");
}

Environment::Environment(const SandboxConfig& sandbox_config)
{
    alloc_info.in_protected_call = false;
    alloc_info.total = 0;
    alloc_info.max = sandbox_config.memory_limit;

    lua = createLuaState(this, luaAlloc, &alloc_info);
    sp2assert(lua, "Failed to create lua state for sandboxed environment. Not giving enough memory for basic state?");
    lua_sethook(lua, luaInstructionCountHook, LUA_MASKCOUNT, sandbox_config.instruction_limit);

    sp2assert(lua_gettop(lua) == 0, "Lua stack incorrect");
}

Environment::~Environment()
{
    //Remove our environment from the registry.
    //REGISTRY[this] = nil
    lua_pushnil(lua);
    lua_rawsetp(lua, LUA_REGISTRYINDEX, this);

    //This assert fails if the environment gets destroyed from a different environments function.
    //sp2assert(lua_gettop(lua) == 0, "Lua stack incorrect");

    destroyLuaState(lua);
}

void Environment::setGlobal(const string& name, lua_CFunction function)
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

void Environment::setGlobal(const string& name, BindingObject* ptr)
{
    //Get the environment table from the registry.
    lua_rawgetp(lua, LUA_REGISTRYINDEX, this);

    //Set our variable in this environment table
    pushToLua(lua, ptr);
    lua_setfield(lua, -2, name.c_str());

    //Pop the table
    lua_pop(lua, 1);
}

void Environment::setGlobal(const string& name, P<BindingObject> ptr)
{
    setGlobal(name, *ptr);
}

void Environment::setGlobal(const string& name, bool value)
{
    //Get the environment table from the registry.
    lua_rawgetp(lua, LUA_REGISTRYINDEX, this);
    
    //Set our variable in this environment table
    lua_pushboolean(lua, value);
    lua_setfield(lua, -2, name.c_str());
    
    //Pop the table
    lua_pop(lua, 1);
}

void Environment::setGlobal(const string& name, int value)
{
    //Get the environment table from the registry.
    lua_rawgetp(lua, LUA_REGISTRYINDEX, this);
    
    //Set our variable in this environment table
    lua_pushinteger(lua, value);
    lua_setfield(lua, -2, name.c_str());
    
    //Pop the table
    lua_pop(lua, 1);
}

void Environment::setGlobal(const string& name, const string& value)
{
    //Get the environment table from the registry.
    lua_rawgetp(lua, LUA_REGISTRYINDEX, this);
    
    //Set our variable in this environment table
    lua_pushstring(lua, value.c_str());
    lua_setfield(lua, -2, name.c_str());
    
    //Pop the table
    lua_pop(lua, 1);
}

void Environment::setGlobal(const string& name, std::nullptr_t)
{
    //Get the environment table from the registry.
    lua_rawgetp(lua, LUA_REGISTRYINDEX, this);
    
    //Set our variable in this environment table
    lua_pushnil(lua);
    lua_setfield(lua, -2, name.c_str());
    
    //Pop the table
    lua_pop(lua, 1);
}

Result<Variant> Environment::load(const string& resource_name)
{
    io::ResourceStreamPtr stream = io::ResourceProvider::get(resource_name);
    if (!stream)
    {
        return Result<Variant>::makeError("Failed to find script resource:" + resource_name);
    }
    return _load(stream, resource_name);
}

Result<Variant> Environment::load(io::ResourceStreamPtr resource)
{
    return _load(resource, "=[resource]");
}

Result<CoroutinePtr> Environment::loadCoroutine(const string& resource_name)
{
    io::ResourceStreamPtr stream = io::ResourceProvider::get(resource_name);
    if (!stream)
    {
        return Result<CoroutinePtr>::makeError("Failed to find script resource:" + resource_name);
    }
    auto code = stream->readAll();
    
    lua_State* L = lua_newthread(lua);

    alloc_info.in_protected_call = true;
    int result = luaL_loadbufferx(L, code.c_str(), code.length(), ("@" + resource_name).c_str(), "t");
    alloc_info.in_protected_call = false;
    if (result)
    {
        auto res = Result<CoroutinePtr>::makeError(lua_tostring(L, -1));
        lua_pop(L, 1);
        lua_pop(lua, 1);
        return res;
    }

    //Get the environment table from the registry.
    lua_rawgetp(L, LUA_REGISTRYINDEX, this);
    //set the environment table it as 1st upvalue
    lua_setupvalue(L, -2, 1);

    return callCoroutineInternal(L, 0);
}

Result<Variant> Environment::run(const string& code)
{
    return _run(code, "=[string]");
}

Result<CoroutinePtr> Environment::runCoroutine(const string& code)
{
    lua_State* L = lua_newthread(lua);

    alloc_info.in_protected_call = true;
    int result = luaL_loadbufferx(L, code.c_str(), code.length(), "=[string]", "t");
    alloc_info.in_protected_call = false;
    if (result)
    {
        auto res = Result<CoroutinePtr>::makeError(lua_tostring(L, -1));
        lua_pop(L, 1);
        lua_pop(lua, 1);
        return res;
    }

    //Get the environment table from the registry.
    lua_rawgetp(L, LUA_REGISTRYINDEX, this);
    //set the environment table it as 1st upvalue
    lua_setupvalue(L, -2, 1);

    return callCoroutineInternal(L, 0);
}


Result<Variant> Environment::_load(io::ResourceStreamPtr resource, const string& name)
{
    if (!resource)
        return Result<Variant>::makeError("No resource provided");

    return _run(resource->readAll(), "@" + name);
}

Result<Variant> Environment::_run(const string& code, const string& name)
{
    alloc_info.in_protected_call = true;
    int result = luaL_loadbufferx(lua, code.c_str(), code.length(), name.c_str(), "t");
    alloc_info.in_protected_call = false;
    if (result)
    {
        auto res = Result<Variant>::makeError(luaL_checkstring(lua, -1));
        lua_pop(lua, 1);
        return res;
    }

    //Get the environment table from the registry.
    lua_rawgetp(lua, LUA_REGISTRYINDEX, this);
    //set the environment table it as 1st upvalue
    lua_setupvalue(lua, -2, 1);

    return callInternal(0);
}

}//namespace script
}//namespace sp
