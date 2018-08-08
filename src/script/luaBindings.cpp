#include <sp2/script/luaBindings.h>
#include <sp2/logging.h>
#include <sp2/assert.h>

namespace sp {
namespace script {

lua_State* global_lua_state;

static int luaLogFunction(lua_State* L)
{
    sp::string log_line;
    int count = lua_gettop(L);  /* number of arguments */
    lua_getglobal(L, "tostring");
    for (int index=1; index<=count; index++)
    {
        lua_pushvalue(L, -1);  /* function to be called */
        lua_pushvalue(L, index);   /* value to print */
        lua_call(L, 1, 1);
        size_t size;
        const char* s = lua_tolstring(L, -1, &size);  /* get result */
        if (s == nullptr)
            return luaL_error(L, "'tostring' must return a string to 'print'");
        if (index > 1)
            log_line += "\t";
        log_line += s;
        lua_pop(L, 1);  /* pop result */
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

int pushToLua(lua_State* L, bool b)
{
    lua_pushboolean(L, b);
    return 1;
}

int pushToLua(lua_State* L, int i)
{
    lua_pushinteger(L, i);
    return 1;
}

int pushToLua(lua_State* L, float f)
{
    lua_pushnumber(L, f);
    return 1;
}

int pushToLua(lua_State* L, double f)
{
    lua_pushnumber(L, f);
    return 1;
}

int pushToLua(lua_State* L, const string& str)
{
    lua_pushstring(L, str.c_str());
    return 1;
}

};//namespace script
};//namespace sp
