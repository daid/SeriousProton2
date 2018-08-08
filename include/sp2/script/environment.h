#ifndef SP2_SCRIPT_ENVIRONMENT_H
#define SP2_SCRIPT_ENVIRONMENT_H

#include <sp2/pointer.h>
#include <sp2/string.h>
#include <sp2/io/resourceProvider.h>
#include <sp2/script/bindingObject.h>
#include <sp2/script/coroutine.h>

namespace sp {
namespace script {

class Environment : public AutoPointerObject
{
public:
    Environment();
    virtual ~Environment();
    
    /** Bind a C function to lua. Use with care, as you need to manage the lua stack yourself */
    void setGlobal(string name, lua_CFunction function);
    /** Bind an object to this lua environment. */
    void setGlobal(string name, P<ScriptBindingObject> ptr);
    
    template<typename RET, typename... ARGS> void setGlobal(string name, RET(*func)(ARGS...))
    {
        typedef RET(*FT)(ARGS...);

        //Get the environment table from the registry.
        lua_rawgetp(global_lua_state, LUA_REGISTRYINDEX, this);
        
        FT* f = reinterpret_cast<FT*>(lua_newuserdata(sp::script::global_lua_state, sizeof(FT)));
        *f = func;
        
        lua_pushcclosure(sp::script::global_lua_state, &script::callFunction<RET, ARGS...>, 1);
        lua_setfield(sp::script::global_lua_state, -2, name.c_str());
        
        //Pop the table
        lua_pop(global_lua_state, 1);
    }
    
    bool load(sp::string resource_name);
    bool load(sp::io::ResourceStreamPtr resource);
    
    template<typename... ARGS> bool call(string global_function, ARGS... args)
    {
        //Get the environment table from the registry.
        lua_rawgetp(global_lua_state, LUA_REGISTRYINDEX, this);

        lua_getfield(global_lua_state, -1, global_function.c_str());
        
        if (lua_isfunction(global_lua_state, -1))
        {
            int arg_count = pushArgs(global_lua_state, args...);
            if (lua_pcall(global_lua_state, arg_count, 0, 0))
            {
                LOG(Error, "Function call error:", global_function, ":", lua_tostring(global_lua_state, -1));
                lua_pop(global_lua_state, 2);
                return false;
            }
            lua_pop(global_lua_state, 1);
            return true;
        }
        lua_pop(global_lua_state, 2);
        return false;
    }

    /** Run a function as coroutine.
        Coroutine functions can be yielded and resumed later.
        While they are yielded, other lua functions can run.
        This makes coroutines perfect for scripted sequences.
     */
    template<typename... ARGS> CoroutinePtr callCoroutine(string global_function, ARGS... args)
    {
        //Get the environment table from the registry.
        lua_rawgetp(global_lua_state, LUA_REGISTRYINDEX, this);
        lua_getfield(global_lua_state, -1, global_function.c_str());
        
        if (!lua_isfunction(global_lua_state, -1))
        {
            lua_pop(global_lua_state, 2);
            return nullptr;
        }
        
        lua_State* L = lua_newthread(global_lua_state);
        lua_pushvalue(global_lua_state, -2);
        lua_xmove(global_lua_state, L, 1);
        int arg_count = pushArgs(L, args...);
        int result = lua_resume(L, nullptr, arg_count);
        if (result != LUA_OK && result != LUA_YIELD)
        {
            LOG(Error, "Function call error:", global_function, ":", lua_tostring(L, -1));
            lua_pop(global_lua_state, 3); //remove environment, function and coroutine
            return nullptr;
        }
        if (result == LUA_OK) //Coroutine didn't yield. So no state to store for it.
        {
            lua_pop(global_lua_state, 3); //remove environment, function and coroutine
            return nullptr;
        }
        std::shared_ptr<Coroutine> coroutine = std::make_shared<Coroutine>(L);
        lua_pop(global_lua_state, 2); //remove environment, function, coroutine is removed by constructor of Coroutine object.
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
