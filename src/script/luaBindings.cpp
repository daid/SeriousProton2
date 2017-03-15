#include <sp2/script/luaBindings.h>
#include <sp2/assert.h>

namespace sp {
namespace script {

lua_State* global_lua_state;

void createGlobalLuaState()
{
    sp2assert(global_lua_state == nullptr, "createGlobalLuaState should only be called once");

    global_lua_state = luaL_newstate();
    luaopen_base(global_lua_state);
    luaL_newmetatable(global_lua_state, "lazyLoading");
    lua_pushstring(global_lua_state, "__index");
    lua_pushcfunction(global_lua_state, lazyLoading);
    lua_settable(global_lua_state, -3);
    lua_pop(global_lua_state, 1);
}

int pushToLua(bool b)
{
    lua_pushboolean(global_lua_state, b);
    return 1;
}

int pushToLua(int i)
{
    lua_pushinteger(global_lua_state, i);
    return 1;
}

int pushToLua(float f)
{
    lua_pushnumber(global_lua_state, f);
    return 1;
}

int pushToLua(double f)
{
    lua_pushnumber(global_lua_state, f);
    return 1;
}

};//!namespace script
};//!namespace sp
