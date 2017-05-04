#ifndef SP2_SCRIPT_LUA_BINDINGS_H
#define SP2_SCRIPT_LUA_BINDINGS_H

#include <lua/lua.hpp>
#include <sp2/pointer.h>
#include <sp2/string.h>
#include <sp2/math/vector.h>
#include <tuple>

namespace sp {
class ScriptBindingObject;
namespace script {

void createGlobalLuaState();

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

int pushToLua(bool b);
int pushToLua(int i);
int pushToLua(float f);
int pushToLua(double f);

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

template<typename T> int pushToLua(sf::Vector2<T> f)
{
    int x = pushToLua(f.x);
    int y = pushToLua(f.y);
    return x + y;
}

template<class TYPE, typename RET> class callClassHelper
{
public:
    template<typename... ARGS, std::size_t... N> static int doCall(TYPE* obj, RET(TYPE::*f)(ARGS...), std::tuple<ARGS...>& args, sequence<N...>)
    {
        return pushToLua((obj->*(f))(std::get<N>(args)...));
    }
};

template<class TYPE> class callClassHelper<TYPE, void>
{
public:
    template<typename... ARGS, std::size_t... N> static int doCall(TYPE* obj, void(TYPE::*f)(ARGS...), std::tuple<ARGS...>& args, sequence<N...>)
    {
        (obj->*(f))(std::get<N>(args)...);
        return pushToLua(obj);
    }
};

template<typename RET> class callFunctionHelper
{
public:
    template<typename... ARGS, std::size_t... N> static int doCall(RET(*f)(ARGS...), std::tuple<ARGS...>& args, sequence<N...>)
    {
        return pushToLua((*f)(std::get<N>(args)...));
    }
};

template<> class callFunctionHelper<void>
{
public:
    template<typename... ARGS, std::size_t... N> static int doCall(void(*f)(ARGS...), std::tuple<ARGS...>& args, sequence<N...>)
    {
        (*f)(std::get<N>(args)...);
        return 0;
    }
};

template<class TYPE, typename RET, typename... ARGS> int callMember(lua_State*)
{
    typedef RET(TYPE::*FT)(ARGS...);
    FT* f = reinterpret_cast<FT*>(lua_touserdata(global_lua_state, lua_upvalueindex(1)));
    TYPE* obj = convertFromLua(typeIdentifier<TYPE*>{}, lua_upvalueindex(2));
    if (!obj)
        return 0;
    std::tuple<ARGS...> args = getArgs<ARGS...>();
    return callClassHelper<TYPE, RET>::doCall(obj, *f, args, typename sequenceGenerator<sizeof...(ARGS)>::type());
}

template<typename RET, typename... ARGS> int callFunction(lua_State*)
{
    typedef RET(*FT)(ARGS...);
    FT* f = reinterpret_cast<FT*>(lua_touserdata(global_lua_state, lua_upvalueindex(1)));
    std::tuple<ARGS...> args = getArgs<ARGS...>();
    return callFunctionHelper<RET>::doCall(*f, args, typename sequenceGenerator<sizeof...(ARGS)>::type());
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

static inline int convertFromLua(typeIdentifier<int>, int index)
{
    return luaL_checkinteger(global_lua_state, index);
}

static inline float convertFromLua(typeIdentifier<float>, int index)
{
    return luaL_checknumber(global_lua_state, index);
}

static inline double convertFromLua(typeIdentifier<double>, int index)
{
    return luaL_checknumber(global_lua_state, index);
}

static inline string convertFromLua(typeIdentifier<string>, int index)
{
    return luaL_checkstring(global_lua_state, index);
}

};//!namespace script
};//!namespace sp

#endif//SP2_SCRIPT_LUA_BINDINGS_H
