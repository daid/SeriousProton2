#include <sp2/script/coroutine.h>
#include <sp2/assert.h>

namespace sp {
namespace script {

Coroutine::Coroutine(lua_State* L)
: L(L)
{
    //When we enter this constructor, the coroutine is still on the stack of the global_lua_state.
    //So use that to store it in the registry, else the garbage collector will collect the coroutine.
    sp2assert(lua_isthread(global_lua_state, -1), "Something is wrong with setting up the coroutine, coroutine not on stack?");
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
    lua_pushnil(L);
    lua_rawsetp(L, LUA_REGISTRYINDEX, this);
    L = nullptr;
}

int Coroutine::getCurrentLineNumber()
{
    if (!L)
        return -1;

    lua_Debug debug;
    int n=0;
    while(lua_getstack(L, n, &debug) != 0)
    {
        if (lua_getinfo(L, "Sl", &debug))
        {
            if (debug.what[0] != 'C')
                return debug.currentline;
        }
        n++;
    }
    return -1;
}

sp::string Coroutine::getCurrentSource()
{
    if (!L)
        return "";

    lua_Debug debug;
    int n=0;
    while(lua_getstack(L, n, &debug) != 0)
    {
        if (lua_getinfo(L, "Sl", &debug))
        {
            if (debug.what[0] != 'C')
            {
                if (debug.source[0] == '=' || debug.source[0] == '@')
                    return debug.source + 1;
                return debug.source;
            }
        }
        n++;
    }
    return "";
}

};//namespace script
};//namespace sp
