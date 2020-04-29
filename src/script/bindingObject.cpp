#include <sp2/script/bindingObject.h>
#include <sp2/logging.h>
#include <sp2/assert.h>
#include <typeinfo>

namespace sp {
namespace script {

int lazyLoadingIndex(lua_State* L)
{
    lazyLoading(1, L);
    //Return the actual field that was requested by lazyloading.
    lua_gettable(L, 1);
    return 1;
}

int lazyLoadingNewIndex(lua_State* L)
{
    lazyLoading(1, L);
    //Set the actual field that was requested by lazyloading.
    lua_settable(L, 1);
    return 1;
}

static int luaIndexProxy(lua_State* L)
{
    lua_getmetatable(L, 1);
    lua_getfield(L, -1, "proxy_table");
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    if (lua_istable(L, -1))
    {
        lua_getfield(L, -1, "get");
        lua_call(L, 0, 1);
    }
    return 1;
}

static int luaNewIndexProxy(lua_State* L)
{
    lua_getmetatable(L, 1);
    lua_getfield(L, -1, "proxy_table");
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    if (lua_isfunction(L, -1))
        return luaL_error(L, "Tried to assign to object function, which is not allowed");
    if (lua_isboolean(L, -1))
        return luaL_error(L, "Tried to assign to object valid value, which is not allowed");
    if (lua_istable(L, -1))
    {
        lua_getfield(L, -1, "set");
        lua_pushvalue(L, 3);
        lua_call(L, 1, 0);
        return 0;
    }
    lua_pop(L, 3);
    lua_rawset(L, 1);
    return 0;
}


void lazyLoading(int table_index, lua_State* L)
{
    //Get the object reference for this object.
    lua_getmetatable(L, table_index);
    lua_getfield(L, -1, "object_ptr");
    BindingObject* sbc = static_cast<BindingObject*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    //Create a new table to store functions and properties for this object.
    lua_newtable(L);
    int function_table_index = lua_gettop(L);
    //Put a field "valid" in this metatable that is always true. (We clear the metatable on object destruction, causing valid to become "nil" and thus false)
    lua_pushboolean(L, true);
    lua_setfield(L, function_table_index, "valid");
    
    //Call the onRegisterScriptBindings which will register functions in the current __index table.
    BindingClass script_binding_class(L, table_index, function_table_index);
    sbc->onRegisterScriptBindings(script_binding_class);
    
    //Set the table as a field in the metatable so we can access it from our proxy function.
    lua_setfield(L, -2, "proxy_table");

    //Push our proxy functions for indexing and value assignment.
    lua_pushcfunction(L, luaIndexProxy);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, luaNewIndexProxy);
    lua_setfield(L, -2, "__newindex");

    //Remove the metatable from the stack, it's already assigned to the object table.
    lua_pop(L, 1);
}

BindingObject::BindingObject()
{
}

void BindingObject::registerToLua(lua_State* L)
{
    if (L == lua)
        return;
    sp2assert(lua == nullptr, "Can only register script objects to a single lua sandbox");
    lua = L;

    //Add object to Lua registry, and register the lazy loader. This loads the bindings on first use, so we do not bind objects that we never use from the scripts.
    //REGISTY[this] = {"metatable": { "object_ptr": this, "__index": lazyLoadingIndex, "__newindex": lazyLoadingNewIndex} }
    lua_newtable(lua);
    lua_newtable(lua);
    lua_pushstring(lua, "[object]");
    lua_setfield(lua, -2, "__metatable");
    lua_pushlightuserdata(lua, this);
    lua_setfield(lua, -2, "object_ptr");
    lua_pushcfunction(lua, script::lazyLoadingIndex);
    lua_setfield(lua, -2, "__index");
    lua_pushcfunction(lua, script::lazyLoadingNewIndex);
    lua_setfield(lua, -2, "__newindex");
    lua_setmetatable(lua, -2);
    lua_rawsetp(lua, LUA_REGISTRYINDEX, this);
}

BindingObject::~BindingObject()
{
    if (lua)
    {
        //Clear our pointer reference in our object table
        lua_rawgetp(lua, LUA_REGISTRYINDEX, this);
        //Clear the metatable of this object.
        lua_pushnil(lua);
        lua_setmetatable(lua, -2);
        lua_pop(lua, 1);

        //Remove object from Lua registry
        //REGISTY[this] = nil
        lua_pushnil(lua);
        lua_rawsetp(lua, LUA_REGISTRYINDEX, this);
    }
}

void BindingObject::onRegisterScriptBindings(BindingClass& script_binding_class)
{
}

}//namespace script
}//namespace sp
