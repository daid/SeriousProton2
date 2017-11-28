#include <sp2/script/callback.h>

namespace sp {
namespace script {

Callback::Callback()
{
}

Callback::~Callback()
{
    lua_pushlightuserdata(global_lua_state, this);
    lua_pushnil(global_lua_state);
    lua_settable(global_lua_state, LUA_REGISTRYINDEX);
}

};//!namespace script
};//!namespace sp
