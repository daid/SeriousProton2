#include <sp2/script/coroutine.h>

namespace sp {
namespace script {

Coroutine::Coroutine(lua_State* L)
: L(L)
{
    //When we enter this constructor, the coroutine is still on the stack of the global_lua_state.
    //So use that to store it in the registry, else the garbage collector will collect the coroutine.
    lua_rawsetp(global_lua_state, LUA_REGISTRYINDEX, this);
}

Coroutine::~Coroutine()
{
    release();
}

void Coroutine::release()
{
    if (!L)
        return;
    //Remove the coroutine from the registry, so the garbage collector will remove it it.
    lua_pushnil(global_lua_state);
    lua_rawsetp(global_lua_state, LUA_REGISTRYINDEX, this);
    L = nullptr;
}

};//namespace script
};//namespace sp
