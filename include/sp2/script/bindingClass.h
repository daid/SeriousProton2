#ifndef SP2_SCRIPT_BINDING_CLASS_H
#define SP2_SCRIPT_BINDING_CLASS_H

#include <lua/lua.hpp>
#include <sp2/pointer.h>
#include <sp2/string.h>
#include <sp2/script/luaBindings.h>
#include <sp2/logging.h>

namespace sp {

class ScriptBindingObject;
class ScriptBindingClass
{
public:
    template<class TYPE, typename RET, typename... ARGS> void addFunction(sp::string name, RET(TYPE::*func)(ARGS...))
    {
        typedef RET(TYPE::*FT)(ARGS...);
        
        lua_pushstring(sp::script::global_lua_state, name.c_str());
        FT* f = reinterpret_cast<FT*>(lua_newuserdata(sp::script::global_lua_state, sizeof(FT)));
        *f = func;
        lua_pushvalue(sp::script::global_lua_state, 1); //push the table of this object
        
        lua_pushcclosure(sp::script::global_lua_state, &script::call<TYPE, RET, ARGS...>, 2);
        lua_settable(sp::script::global_lua_state, -3);
    }

private:
    ScriptBindingClass() {}
    
    friend int script::lazyLoading(lua_State* L);
};

};//!namespace sp

#endif//SP2_SCRIPT_BINDING_OBJECT_H

