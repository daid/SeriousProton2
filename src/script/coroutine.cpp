#include <sp2/script/coroutine.h>
#include <sp2/script/environment.h>
#include <sp2/assert.h>

namespace sp {
namespace script {

Coroutine::Coroutine(P<Environment> environment, lua_State* origin, lua_State* L)
: L(L), environment(environment)
{
    //When we enter this constructor, the coroutine is still on the stack of the global_lua_state.
    //So use that to store it in the registry, else the garbage collector will collect the coroutine.
    sp2assert(lua_isthread(origin, -1), "Something is wrong with setting up the coroutine, coroutine not on stack?");
    lua_rawsetp(origin, LUA_REGISTRYINDEX, this);
}

Coroutine::~Coroutine()
{
    release();
}

bool Coroutine::internalResume(int arg_count)
{
    //Set the hook as it was already, so the internal counter gets reset for sandboxed environments.
    lua_sethook(L, lua_gethook(L), lua_gethookmask(L), lua_gethookcount(L));
    environment->alloc_info.in_protected_call = true;
    int status = lua_resume(L, nullptr, arg_count);
    environment->alloc_info.in_protected_call = false;

    if (status == LUA_YIELD)
        return true;
    if (status != LUA_OK)
    {
        last_error = lua_tostring(L, -1);
    }
    release();
    return false;
}

void Coroutine::release()
{
    if (!environment || !L)
        return;
    //Remove the coroutine from the registry, so the garbage collector will remove it it.
    lua_pushnil(L);
    lua_rawsetp(L, LUA_REGISTRYINDEX, this);
    L = nullptr;
    environment = nullptr;
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

string Coroutine::getCurrentSource()
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

}//namespace script
}//namespace sp
