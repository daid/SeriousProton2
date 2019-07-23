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
    class SandboxConfig
    {
    public:
        size_t memory_limit;
        int instruction_limit;
    };

    Environment();
    Environment(const SandboxConfig& sandbox_config);
    virtual ~Environment();
    
    /** Bind a C function to lua. Use with care, as you need to manage the lua stack yourself */
    void setGlobal(string name, lua_CFunction function);
    /** Bind an object to this lua environment. */
    void setGlobal(string name, ScriptBindingObject* ptr);
    void setGlobal(string name, P<ScriptBindingObject> ptr);
    void setGlobal(string name, bool value);
    void setGlobal(string name, int value);
    void setGlobal(string name, string value);
    
    template<typename RET, typename... ARGS> void setGlobal(string name, RET(*func)(ARGS...))
    {
        typedef RET(*FT)(ARGS...);

        //Get the environment table from the registry.
        lua_rawgetp(lua, LUA_REGISTRYINDEX, this);
        
        FT* f = reinterpret_cast<FT*>(lua_newuserdata(lua, sizeof(FT)));
        *f = func;
        
        lua_pushcclosure(lua, &script::callFunction<RET, ARGS...>, 1);
        lua_setfield(lua, -2, name.c_str());
        
        //Pop the table
        lua_pop(lua, 1);
    }
    
    bool load(const string& resource_name);
    bool load(sp::io::ResourceStreamPtr resource);
    bool run(const string& code);
    CoroutinePtr runCoroutine(const string& code);
    
    //Call a script function. Return true if the call was made, false on an error.
    template<typename... ARGS> bool call(string global_function, ARGS... args)
    {
        //Get the environment table from the registry.
        lua_rawgetp(lua, LUA_REGISTRYINDEX, this);

        lua_getfield(lua, -1, global_function.c_str());
        
        last_error = "";
        if (lua_isfunction(lua, -1))
        {
            int arg_count = pushArgs(lua, args...);

            //Set the hook as it was already, so the internal counter gets reset for sandboxed environments.
            lua_sethook(lua, lua_gethook(lua), lua_gethookmask(lua), lua_gethookcount(lua));

            if (lua_pcall(lua, arg_count, 0, 0))
            {
                last_error = lua_tostring(lua, -1);
                LOG(Error, "Function call error:", global_function, ":", last_error);
                lua_pop(lua, 2);
                return false;
            }
            lua_pop(lua, 1);
            return true;
        }
        lua_pop(lua, 2);
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
        lua_rawgetp(lua, LUA_REGISTRYINDEX, this);
        lua_getfield(lua, -1, global_function.c_str());
        
        last_error = "";
        if (!lua_isfunction(lua, -1))
        {
            lua_pop(lua, 2);
            return nullptr;
        }
        
        lua_State* L = lua_newthread(lua);
        lua_pushvalue(lua, -2);
        lua_xmove(lua, L, 1);
        int arg_count = pushArgs(L, args...);
        int result = lua_resume(L, nullptr, arg_count);
        if (result != LUA_OK && result != LUA_YIELD)
        {
            last_error = lua_tostring(L, -1);
            LOG(Error, "Function call error:", global_function, ":", last_error);
            lua_pop(lua, 3); //remove environment, function and coroutine
            return nullptr;
        }
        if (result == LUA_OK) //Coroutine didn't yield. So no state to store for it.
        {
            lua_pop(lua, 3); //remove environment, function and coroutine
            return nullptr;
        }
        std::shared_ptr<Coroutine> coroutine = std::make_shared<Coroutine>(L);
        lua_pop(lua, 2); //remove environment, function, coroutine is removed by constructor of Coroutine object.
        return coroutine;
    }

    const sp::string& getLastError()
    {
        return last_error;
    }

    struct AllocInfo
    {
        size_t total;
        size_t max;
    };
private:
    bool _load(io::ResourceStreamPtr resource, const string& name);
    bool _run(const string& code, const string& name);

    int pushArgs(lua_State* L)
    {
        return 0;
    }

    template<typename ARG, typename... ARGS> int pushArgs(lua_State* L, ARG arg, ARGS... args)
    {
        pushToLua(L, arg);
        return 1 + pushArgs(L, args...);
    }
    
    lua_State* lua;
    sp::string last_error;
    AllocInfo alloc_info;
};

};//namespace script
};//namespace sp

#endif//SP2_SCRIPT_ENVIRONMENT_H
