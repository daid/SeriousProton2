#ifndef SP2_SCRIPT_CALLBACK_H
#define SP2_SCRIPT_CALLBACK_H

#include <sp2/pointer.h>
#include <sp2/string.h>
#include <sp2/io/resourceProvider.h>
#include <sp2/script/bindingObject.h>

namespace sp {
namespace script {

class Callback : public sf::NonCopyable
{
public:
    Callback();
    ~Callback();
    
    template<typename... ARGS> bool call(ARGS... args)
    {
        lua_pushlightuserdata(global_lua_state, this);
        lua_gettable(global_lua_state, LUA_REGISTRYINDEX);
        if (lua_isfunction(global_lua_state, -1))
        {
            if (lua_pcall(global_lua_state, 0, 0, 0))
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
};

};//!namespace script
};//!namespace sp

#endif//SP2_SCRIPT_ENVIRONMENT_H
