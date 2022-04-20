#ifndef SP2_SCRIPT_ENVIRONMENT_H
#define SP2_SCRIPT_ENVIRONMENT_H

#include <sp2/pointer.h>
#include <sp2/result.h>
#include <sp2/io/resourceProvider.h>
#include <sp2/script/bindingObject.h>
#include <sp2/script/luaState.h>

namespace sp {
namespace script {

class Environment : public AutoPointerObject, public LuaState
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
    void setGlobal(const string& name, lua_CFunction function);
    /** Bind an object to this lua environment. */
    void setGlobal(const string& name, BindingObject* ptr);
    void setGlobal(const string& name, P<BindingObject> ptr);
    void setGlobal(const string& name, bool value);
    void setGlobal(const string& name, int value);
    void setGlobal(const string& name, const string& value);

    template<typename RET, typename... ARGS> void setGlobal(const string& name, RET(*func)(ARGS...))
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

    Result<Variant> load(const string& resource_name);
    Result<Variant> load(io::ResourceStreamPtr resource);
    Result<CoroutinePtr> loadCoroutine(const string& resource_name);
    Result<Variant> run(const string& code);
    Result<CoroutinePtr> runCoroutine(const string& code);

    //Call a script function. Return true if the call was made, false on an error.
    template<typename... ARGS> Result<Variant> call(const string& global_function, ARGS... args)
    {
        //Get the environment table from the registry.
        lua_rawgetp(lua, LUA_REGISTRYINDEX, this);

        lua_getfield(lua, -1, global_function.c_str());
        lua_remove(lua, -2);

        if (lua_isfunction(lua, -1))
        {
            int arg_count = pushArgs(lua, args...);
            return callInternal(arg_count);
        }
        sp::string err = global_function + " is not a function but: " + luaL_typename(lua, -1);
        lua_pop(lua, 1);
        return Result<Variant>::makeError(std::move(err));
    }

    /** Run a function as coroutine.
        Coroutine functions can be yielded and resumed later.
        While they are yielded, other lua functions can run.
        This makes coroutines perfect for scripted sequences.
     */
    template<typename... ARGS> Result<CoroutinePtr> callCoroutine(const string& global_function, ARGS... args)
    {
        //Get the environment table from the registry.
        lua_rawgetp(lua, LUA_REGISTRYINDEX, this);
        lua_getfield(lua, -1, global_function.c_str());
        lua_remove(lua, -2);

        if (!lua_isfunction(lua, -1))
        {
            sp::string err = global_function + " is not a function but: " + luaL_typename(lua, -1);
            lua_pop(lua, 1);
            return Result<CoroutinePtr>::makeError(std::move(err));
        }

        lua_State* L = lua_newthread(lua);
        lua_rotate(lua, -2, 1);
        lua_xmove(lua, L, 1);
        int arg_count = pushArgs(L, args...);
        return callCoroutineInternal(L, arg_count);
    }

    struct AllocInfo
    {
        bool in_protected_call;
        size_t total;
        size_t max;
    };
private:
    Result<Variant> _load(io::ResourceStreamPtr resource, const string& name);
    Result<Variant> _run(const string& code, const string& name);

    AllocInfo alloc_info;

    friend class Coroutine;
    friend class Callback;
};

}//namespace script
}//namespace sp

#endif//SP2_SCRIPT_ENVIRONMENT_H
