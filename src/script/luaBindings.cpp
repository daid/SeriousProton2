#include <sp2/script/luaBindings.h>
#include <sp2/logging.h>
#include <sp2/assert.h>

namespace sp {
namespace script {

lua_State* global_lua_state;

static int luaLogFunction(lua_State* lua)
{
    sp::string log_line;
    int count = lua_gettop(lua);  /* number of arguments */
    lua_getglobal(lua, "tostring");
    for (int index=1; index<=count; index++)
    {
        lua_pushvalue(lua, -1);  /* function to be called */
        lua_pushvalue(lua, index);   /* value to print */
        lua_call(lua, 1, 1);
        size_t size;
        const char* s = lua_tolstring(lua, -1, &size);  /* get result */
        if (s == nullptr)
            return luaL_error(lua, "'tostring' must return a string to 'print'");
        if (index > 1)
            log_line += "\t";
        log_line += s;
        lua_pop(lua, 1);  /* pop result */
    }
    LOG(Info, log_line);
    return 0;
}

void addVectorMetatables();
void createGlobalLuaState()
{
    sp2assert(global_lua_state == nullptr, "createGlobalLuaState should only be called once");

    global_lua_state = luaL_newstate();

    luaL_requiref(global_lua_state, "_G", luaopen_base, true);
    lua_pop(global_lua_state, 1);
    luaL_requiref(global_lua_state, LUA_TABLIBNAME, luaopen_table, true);
    lua_pop(global_lua_state, 1);
    luaL_requiref(global_lua_state, LUA_STRLIBNAME, luaopen_string, true);
    lua_pop(global_lua_state, 1);
    luaL_requiref(global_lua_state, LUA_MATHLIBNAME, luaopen_math, true);
    lua_pop(global_lua_state, 1);

    //Override the print function from "base" with our own log function.
    lua_register(global_lua_state, "print", luaLogFunction);
    lua_register(global_lua_state, "log", luaLogFunction);

    luaL_newmetatable(global_lua_state, "lazyLoading");
    lua_pushstring(global_lua_state, "__index");
    lua_pushcfunction(global_lua_state, lazyLoading);
    lua_settable(global_lua_state, -3);
    lua_pop(global_lua_state, 1);

    addVectorMetatables();
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

};//namespace script
};//namespace sp
