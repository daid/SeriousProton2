#include <sp2/script/callback.h>

namespace sp {
namespace script {

Callback::Callback()
{
}

Callback::~Callback()
{
    lua_pushnil(global_lua_state);
    lua_rawsetp(global_lua_state, LUA_REGISTRYINDEX, this);
}

};//namespace script
};//namespace sp
