#include <sp2/script/luaState.h>
#include <sp2/script/environment.h>
#include <sp2/assert.h>


namespace sp {
namespace script {

bool LuaState::callInternal(int arg_count)
{
    setLastError("");
    Environment::AllocInfo* alloc_info;
    lua_getallocf(lua, reinterpret_cast<void**>(&alloc_info));
    if (alloc_info)
    {
        alloc_info->in_protected_call = true;
        //Set the hook as it was already, so the internal counter gets reset for sandboxed environments.
        lua_sethook(lua, lua_gethook(lua), lua_gethookmask(lua), lua_gethookcount(lua));
    }
    int result = lua_pcall(lua, arg_count, 0, 0);
    if (alloc_info)
        alloc_info->in_protected_call = false;
    if (result)
    {
        setLastError(lua_tostring(lua, -1));
        lua_pop(lua, 1);
        return false;
    }
    return true;
}

bool LuaState::resumeInternal(int arg_count)
{
    setLastError("");
    Environment::AllocInfo* alloc_info;
    lua_getallocf(lua, reinterpret_cast<void**>(&alloc_info));
    if (alloc_info)
    {
        alloc_info->in_protected_call = true;
        //Set the hook as it was already, so the internal counter gets reset for sandboxed environments.
        lua_sethook(lua, lua_gethook(lua), lua_gethookmask(lua), lua_gethookcount(lua));
    }
    int result = lua_resume(lua, nullptr, arg_count);
    if (alloc_info)
        alloc_info->in_protected_call = false;
    if (result == LUA_YIELD)
        return true;
    if (result != LUA_OK)
    {
        setLastError(lua_tostring(lua, -1));
        lua_pop(lua, 1);
    }
    return false;
}

CoroutinePtr LuaState::callCoroutineInternal(lua_State* L, int arg_count)
{
    setLastError("");
    Environment::AllocInfo* alloc_info;
    lua_getallocf(L, reinterpret_cast<void**>(&alloc_info));
    if (alloc_info)
    {
        alloc_info->in_protected_call = true;
        //Set the hook as it was already, so the internal counter gets reset for sandboxed environments.
        lua_sethook(L, lua_gethook(L), lua_gethookmask(L), lua_gethookcount(L));
    }
    int result = lua_resume(L, nullptr, arg_count);
    if (alloc_info)
        alloc_info->in_protected_call = false;
    if (result == LUA_YIELD)
        return std::make_shared<Coroutine>(lua, L);

    if (result != LUA_OK && result != LUA_YIELD)
        setLastError(lua_tostring(L, -1));
    //Coroutine didn't yield. So no state to store for it.
    lua_pop(lua, 1);//pop the coroutine off the main stack.
    return nullptr;
}

void LuaState::setLastError(string error)
{
    last_error = std::move(error);
    if (!last_error.empty())
        LOG(Error, "Lua:", last_error);
}

}//namespace script
}//namespace sp
