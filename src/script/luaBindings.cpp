#include <sp2/script/luaBindings.h>
#include <sp2/script/bindingObject.h>
#include <sp2/logging.h>
#include <sp2/assert.h>

namespace sp {
namespace script {

static lua_State* global_lua_state;

static int luaLogFunctionInternal(lua_State* L)
{
    string log_line;
    int count = lua_gettop(L);  /* number of arguments */
    lua_getglobal(L, "tostring");
    for (int index=1; index<=count; index++)
    {
        lua_pushvalue(L, -1);  /* function to be called */
        lua_pushvalue(L, index);   /* value to print */
        if (lua_pcall(L, 1, 1, 0) != LUA_OK)
            return 1;
        size_t size;
        const char* s = lua_tolstring(L, -1, &size);  /* get result */
        if (s == nullptr)
        {
            lua_pushstring(L, "'tostring' must return a string to 'print'");
            return 1;
        }
        if (index > 1)
            log_line += "\t";
        log_line += s;
        lua_pop(L, 1);  /* pop result */
    }
    LOG(Info, log_line);
    return 0;
}

static int luaLogFunction(lua_State* L)
{
    if (luaLogFunctionInternal(L))
        return lua_error(L);
    return 0;
}

static int panic(lua_State *L)
{
    LOG(Error, "PANIC: unprotected error in call to Lua API (", lua_tostring(L, -1), ")");
    return 0;  /* return to Lua to abort */
}


void addVectorMetatables(lua_State*);
static void setupGlobalFunctions(lua_State* L);
static void createEnvironmentTable(void* environment, lua_State* L, bool shared);

lua_State* createLuaState(void* environment, lua_Alloc alloc_function, void* alloc_ptr)
{
    if (!alloc_function)
    {
        if (!global_lua_state)
        {
            global_lua_state = luaL_newstate();
            setupGlobalFunctions(global_lua_state);
        }
        createEnvironmentTable(environment, global_lua_state, true);
        return global_lua_state;
    }
    auto lua = lua_newstate(alloc_function, alloc_ptr);
    setupGlobalFunctions(lua);
    createEnvironmentTable(environment, lua, false);
    return lua;
}

void destroyLuaState(lua_State* lua)
{
    if (lua != global_lua_state)
        lua_close(lua);
}

static void setupGlobalFunctions(lua_State* lua)
{
    lua_atpanic(lua, &panic);

    luaL_requiref(lua, "_G", luaopen_base, true);
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
    lua_pushnil(lua);
    lua_setglobal(lua, "pcall");
    lua_pushnil(lua);
    lua_setglobal(lua, "xpcall");

    //Override the print function from "base" with our own log function.
    lua_register(lua, "print", luaLogFunction);
    lua_register(lua, "log", luaLogFunction);

    //Add extra libraries
    luaL_requiref(lua, LUA_TABLIBNAME, luaopen_table, true);
    lua_pop(lua, 1);
    luaL_requiref(lua, LUA_STRLIBNAME, luaopen_string, true);
    lua_pop(lua, 1);
    luaL_requiref(lua, LUA_MATHLIBNAME, luaopen_math, true);
    lua_pop(lua, 1);

    addVectorMetatables(lua);
}

static void createEnvironmentTable(void* environment, lua_State* lua, bool shared)
{
    //Create a new lua environment.
    //REGISTY[this] = {"metatable": {"__index": _G, "environment_ptr": this}}
    if (shared)
        lua_newtable(lua); //environment
    else
        lua_pushglobaltable(lua); //environment
    lua_newtable(lua); //environment metatable
    lua_pushstring(lua, "[environment]");
    lua_setfield(lua, -2, "__metatable");
    if (shared)
    {
        lua_pushglobaltable(lua);
        lua_setfield(lua, -2, "__index");
    }
    //Set the ptr in the metatable.
    lua_pushlightuserdata(lua, environment);
    lua_setfield(lua, -2, "environment_ptr");
    lua_setmetatable(lua, -2);
    lua_rawsetp(lua, LUA_REGISTRYINDEX, environment);
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

int pushToLua(lua_State* L, Yield)
{
    return 0;
}

int pushToLua(lua_State* L, BindingObject* ptr)
{
    if (ptr)
    {
        ptr->registerToLua(L);
        lua_pushlightuserdata(L, ptr);
        lua_gettable(L, LUA_REGISTRYINDEX);
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

}//namespace script
}//namespace sp
