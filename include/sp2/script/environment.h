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
        lua_pushlightuserdata(global_lua_state, this);
        lua_gettable(global_lua_state, LUA_REGISTRYINDEX);
        
        lua_pushstring(sp::script::global_lua_state, name.c_str());
        FT* f = reinterpret_cast<FT*>(lua_newuserdata(sp::script::global_lua_state, sizeof(FT)));
        *f = func;
        
        lua_pushcclosure(sp::script::global_lua_state, &script::callFunction<RET, ARGS...>, 1);
        lua_settable(sp::script::global_lua_state, -3);
    }
    
    bool load(sp::io::ResourceStreamPtr resource);
};

};//!namespace script
};//!namespace sp

#endif//SP2_SCRIPT_ENVIRONMENT_H
