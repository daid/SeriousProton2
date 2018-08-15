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
        //Get this callback from the registry
        lua_rawgetp(global_lua_state, LUA_REGISTRYINDEX, this);
        if (lua_isfunction(global_lua_state, -1))
        {
            //If it exists, push the arguments with it, can run it.
            int arg_count = pushArgs(global_lua_state, args...);
            if (lua_pcall(global_lua_state, arg_count, 0, 0))
            {
                LOG(Error, "Callback function error:", lua_tostring(global_lua_state, -1));
                lua_pop(global_lua_state, 1);
                return false;
            }
            return true;
        }
        lua_pop(global_lua_state, 1);
        return false;
    }

    /** Run the callback as coroutine.
        Coroutine functions can be yielded and resumed later.
        While they are yielded, other lua functions can run.
        This makes coroutines perfect for scripted sequences.
     */
    template<typename... ARGS> CoroutinePtr callCoroutine(ARGS... args)
    {
        //Get the callback from the registry.
        lua_rawgetp(global_lua_state, LUA_REGISTRYINDEX, this);
        //If it's not set, then we can ignore it.
        if (!lua_isfunction(global_lua_state, -1))
        {
            lua_pop(global_lua_state, 1);
            return nullptr;
        }
        
        lua_State* L = lua_newthread(global_lua_state);
        lua_pushvalue(global_lua_state, -2);
        lua_xmove(global_lua_state, L, 1);
        int arg_count = pushArgs(L, args...);
        int result = lua_resume(L, nullptr, arg_count);
        if (result != LUA_OK && result != LUA_YIELD)
        {
            LOG(Error, "Callback call error:", lua_tostring(L, -1));
            lua_pop(global_lua_state, 2); //remove function and coroutine
            return nullptr;
        }
        if (result == LUA_OK) //Coroutine didn't yield. So no state to store for it.
        {
            lua_pop(global_lua_state, 2); //remove function and coroutine
            return nullptr;
        }
        std::shared_ptr<Coroutine> coroutine = std::make_shared<Coroutine>(L);
        lua_pop(global_lua_state, 1); //remove function, coroutine is removed by constructor of Coroutine object.
        return coroutine;
    }

private:
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

};//namespace script
};//namespace sp

#endif//SP2_SCRIPT_ENVIRONMENT_H
