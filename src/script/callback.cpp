#include <sp2/script/callback.h>
#include <sp2/script/environment.h>


namespace sp {
namespace script {

Callback::Callback()
{
}

Callback::~Callback()
{
    if (lua)
    {
        lua_pushnil(lua);
        lua_rawsetp(lua, LUA_REGISTRYINDEX, this);
    }
}

}//namespace script
}//namespace sp
