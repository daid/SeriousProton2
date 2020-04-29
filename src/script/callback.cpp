#include <sp2/script/callback.h>
#include <sp2/script/environment.h>


namespace sp {
namespace script {

Callback::Callback()
{
}

Callback::~Callback()
{
    if (lua)
    {
        lua_pushnil(lua);
        lua_rawsetp(lua, LUA_REGISTRYINDEX, this);
    }
}

bool Callback::callInternal(int arg_count)
{
    environment->alloc_info.in_protected_call = true;
    int result = lua_pcall(lua, arg_count, 0, 0);
    environment->alloc_info.in_protected_call = false;
    if (result)
    {
        LOG(Error, "Callback function error:", lua_tostring(lua, -1));
        lua_pop(lua, 1);
        return false;
    }
    return true;
}

CoroutinePtr Callback::callCoroutineInternal(lua_State* L, int arg_count)
{
    environment->alloc_info.in_protected_call = true;
    int result = lua_resume(L, nullptr, arg_count);
    environment->alloc_info.in_protected_call = false;
    if (result != LUA_OK && result != LUA_YIELD)
    {
        LOG(Error, "Callback call error:", lua_tostring(L, -1));
        lua_pop(lua, 2); //remove function and coroutine
        return nullptr;
    }
    if (result == LUA_OK) //Coroutine didn't yield. So no state to store for it.
    {
        lua_pop(lua, 2); //remove function and coroutine
        return nullptr;
    }

    std::shared_ptr<Coroutine> coroutine = std::make_shared<Coroutine>(environment, lua, L);
    lua_pop(lua, 1); //remove function, coroutine is removed by constructor of Coroutine object.
    return coroutine;
}

}//namespace script
}//namespace sp
