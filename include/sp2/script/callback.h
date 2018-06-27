#ifndef SP2_SCRIPT_CALLBACK_H
#define SP2_SCRIPT_CALLBACK_H

#include <sp2/pointer.h>
#include <sp2/string.h>
#include <sp2/io/resourceProvider.h>
#include <sp2/script/bindingObject.h>

namespace sp {
namespace script {

class Callback : public NonCopyable
{
public:
    Callback();
    ~Callback();
    
    template<typename... ARGS> bool call(ARGS... args)
    {
        lua_rawgetp(global_lua_state, LUA_REGISTRYINDEX, this);
        if (lua_isfunction(global_lua_state, -1))
        {
            int arg_count = pushArgs(args...);
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
