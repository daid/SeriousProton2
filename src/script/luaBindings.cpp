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

static int panic(lua_State *L)
{
    LOG(Error, "PANIC: unprotected error in call to Lua API (", lua_tostring(L, -1), ")");
    return 0;  /* return to Lua to abort */
}


void addVectorMetatables(lua_State*);
lua_State* createLuaState()
{
    lua_State* lua = luaL_newstate();
    lua_atpanic(lua, &panic);

    luaL_requiref(lua, "_G", luaopen_base, true);
    lua_pop(lua, 1);
    luaL_requiref(lua, LUA_TABLIBNAME, luaopen_table, true);
    lua_pop(lua, 1);
    luaL_requiref(lua, LUA_STRLIBNAME, luaopen_string, true);
    lua_pop(lua, 1);
    luaL_requiref(lua, LUA_MATHLIBNAME, luaopen_math, true);
    lua_pop(lua, 1);
    
    //Remove unsafe base functions.
    lua_pushnil(lua);
    lua_setglobal(lua, "collectgarbage");
    lua_pushnil(lua);
    lua_setglobal(lua, "dofile");
    lua_pushnil(lua);
    lua_setglobal(lua, "getmetatable");
    lua_pushnil(lua);
    lua_setglobal(lua, "loadfile");
    lua_pushnil(lua);
    lua_setglobal(lua, "load");
    lua_pushnil(lua);
    lua_setglobal(lua, "rawequal");
    lua_pushnil(lua);
    lua_setglobal(lua, "setmetatable");

    //Override the print function from "base" with our own log function.
    lua_register(lua, "print", luaLogFunction);
    lua_register(lua, "log", luaLogFunction);
    
    lua_pop(lua, 1);

    addVectorMetatables(lua);
    return lua;
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
