#ifndef SP2_SCRIPT_LUA_BINDINGS_H
#define SP2_SCRIPT_LUA_BINDINGS_H

#include <lua/lua.hpp>
#include <sp2/pointer.h>
#include <sp2/string.h>

namespace sp {
class ScriptBindingObject;
namespace script {

int lazyLoading(lua_State* L);
extern lua_State* global_lua_state;

template<typename T> struct typeIdentifier{};
template<std::size_t ...> struct sequence{};
template<std::size_t N, std::size_t ...S> struct sequenceGenerator : sequenceGenerator<N-1, N-1, S...>{};
template<std::size_t ...S> struct sequenceGenerator<0, S...>{ typedef sequence<S...> type; };

template<typename... ARGS, std::size_t... N> std::tuple<ARGS...> getArgs(sequence<N...>)
{
    return std::tuple<ARGS...>{convertFromLua(typeIdentifier<ARGS>{}, N + 1)...};
}

template<typename... ARGS> std::tuple<ARGS...> getArgs()
{
    return getArgs<ARGS...>(typename sequenceGenerator<sizeof... (ARGS)>::type());
}

template<class T> int pushToLua(sp::P<T> obj)
{
    if (obj)
    {
        sp::ScriptBindingObject* ptr = *obj;
        lua_pushlightuserdata(global_lua_state, ptr);
        lua_gettable(global_lua_state, LUA_REGISTRYINDEX);
        return 1;
    }
    lua_pushnil(global_lua_state);
    return 1;
}

template<class T> int pushToLua(T* obj)
{
    if (obj)
    {
        sp::ScriptBindingObject* ptr = obj;
        lua_pushlightuserdata(global_lua_state, ptr);
        lua_gettable(global_lua_state, LUA_REGISTRYINDEX);
        return 1;
    }
    lua_pushnil(global_lua_state);
    return 1;
}

template<class TYPE, typename RET> class callClass
{
public:
    template<typename... ARGS, std::size_t... N> static int doCall(TYPE* obj, RET(TYPE::*f)(ARGS...), std::tuple<ARGS...>& args, sequence<N...>)
    {
        return pushToLua((obj->*(f))(std::get<N>(args)...));
    }
};

template<class TYPE> class callClass<TYPE, void>
{
public:
    template<typename... ARGS, std::size_t... N> static int doCall(TYPE* obj, void(TYPE::*f)(ARGS...), std::tuple<ARGS...>& args, sequence<N...>)
    {
        (obj->*(f))(std::get<N>(args)...);
        return pushToLua(obj);
    }
};

template<class TYPE, typename RET, typename... ARGS> int call(lua_State*)
{
    typedef RET(TYPE::*FT)(ARGS...);
    FT* f = reinterpret_cast<FT*>(lua_touserdata(global_lua_state, lua_upvalueindex(1)));
    TYPE* obj = convertFromLua(typeIdentifier<TYPE*>{}, lua_upvalueindex(2));
    if (!obj)
        return 0;
    std::tuple<ARGS...> args = getArgs();
    return callClass<TYPE, RET>::doCall(obj, *f, args, typename sequenceGenerator<sizeof...(ARGS)>::type());
}

template<typename T> T* convertFromLua(typeIdentifier<T*>, int index)
{
    luaL_checktype(global_lua_state, index, LUA_TTABLE);
    lua_pushstring(global_lua_state, "__ptr");
    lua_gettable(global_lua_state, index);

    T* obj = static_cast<T*>(static_cast<ScriptBindingObject*>(lua_touserdata(global_lua_state, -1)));
    lua_pop(global_lua_state, 1);
    return obj;
}

};//!namespace script
};//!namespace sp

#endif//SP2_SCRIPT_LUA_BINDINGS_H
