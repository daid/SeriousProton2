#ifndef SP2_SCRIPT_CALLBACK_H
#define SP2_SCRIPT_CALLBACK_H

#include <sp2/pointer.h>
#include <sp2/string.h>
#include <sp2/io/resourceProvider.h>
#include <sp2/script/bindingObject.h>
#include <sp2/script/coroutine.h>


namespace sp {
namespace script {

class Callback : public NonCopyable
{
public:
    Callback();
    ~Callback();

    template<typename... ARGS> bool call(ARGS... args)
    {
        if (!lua)
            return false;

        //Get this callback from the registry
        lua_rawgetp(lua, LUA_REGISTRYINDEX, this);
        if (!lua_isfunction(lua, -1))
        {
            lua_pop(lua, 1);
            return false;
        }
        //If it exists, push the arguments with it, can run it.
        return callInternal(pushArgs(lua, args...));
    }

    /** Run the callback as coroutine.
        Coroutine functions can be yielded and resumed later.
        While they are yielded, other lua functions can run.
        This makes coroutines perfect for scripted sequences.
     */
    template<typename... ARGS> CoroutinePtr callCoroutine(ARGS... args)
    {
        if (!lua)
            return nullptr;

        //Get the callback from the registry.
        lua_rawgetp(lua, LUA_REGISTRYINDEX, this);
        //If it's not set, then we can ignore it.
        if (!lua_isfunction(lua, -1))
        {
            lua_pop(lua, 1);
            return nullptr;
        }

        lua_State* L = lua_newthread(lua);
        lua_pushvalue(lua, -2);
        lua_xmove(lua, L, 1);
        return callCoroutineInternal(L, pushArgs(L, args...));
    }

//TODO: These should not be public
    P<Environment> environment;
    lua_State* lua = nullptr;
private:
    bool callInternal(int arg_count);
    CoroutinePtr callCoroutineInternal(lua_State* L, int arg_count);

    int pushArgs(lua_State* L)
    {
        return 0;
    }

    template<typename ARG, typename... ARGS> int pushArgs(lua_State* L, ARG arg, ARGS... args)
    {
        pushToLua(L, arg);
        return 1 + pushArgs(L, args...);
    }
};

}//namespace script
}//namespace sp

#endif//SP2_SCRIPT_ENVIRONMENT_H
