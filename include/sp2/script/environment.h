#ifndef SP2_SCRIPT_ENVIRONMENT_H
#define SP2_SCRIPT_ENVIRONMENT_H

#include <sp2/pointer.h>
#include <sp2/string.h>
#include <sp2/io/resourceProvider.h>
#include <sp2/script/bindingObject.h>

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
