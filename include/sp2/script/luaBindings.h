#ifndef SP2_SCRIPT_LUA_BINDINGS_H
#define SP2_SCRIPT_LUA_BINDINGS_H

#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include <sp2/pointer.h>
#include <sp2/string.h>
#include <sp2/variant.h>
#include <sp2/math/vector.h>
#include <tuple>

namespace sp {
namespace script {
class BindingObject;
// Special dummy object to return to yield the current script execution.
class Yield{};

lua_State* createLuaState(void* environment, lua_Alloc alloc_function=nullptr, void* alloc_ptr=nullptr);
void destroyLuaState(lua_State* L);

int lazyLoadingIndex(lua_State* L);
int lazyLoadingNewIndex(lua_State* L);
void lazyLoading(int table_index, lua_State* L);

template<typename T> struct typeIdentifier{};
template<std::size_t ...> struct sequence{};
template<std::size_t N, std::size_t ...S> struct sequenceGenerator : sequenceGenerator<N-1, N-1, S...>{};
template<std::size_t ...S> struct sequenceGenerator<0, S...>{ typedef sequence<S...> type; };
template<typename T> using remove_cvref = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

template<typename... ARGS, std::size_t... N> std::tuple<ARGS...> getArgs(lua_State* L, sequence<N...>)
{
    return std::tuple<ARGS...>{convertFromLua(L, typeIdentifier<ARGS>{}, N + 1)...};
}

template<typename... ARGS> std::tuple<ARGS...> getArgs(lua_State* L)
{
    return getArgs<ARGS...>(L, typename sequenceGenerator<sizeof... (ARGS)>::type());
}

int pushToLua(lua_State* L, bool b);
int pushToLua(lua_State* L, int i);
int pushToLua(lua_State* L, float f);
int pushToLua(lua_State* L, double f);
int pushToLua(lua_State* L, const string& str);
int pushToLua(lua_State* L, Yield);
int pushToLua(lua_State* L, BindingObject* ptr);

template<class T, class = typename std::enable_if<std::is_base_of<BindingObject, T>::value>::type> int pushToLua(lua_State* L, sp::P<T> obj)
{
    if (obj)
    {
        BindingObject* ptr = *obj;
        return pushToLua(L, ptr);
    }
    lua_pushnil(L);
    return 1;
}

template<class T, class = typename std::enable_if<std::is_base_of<BindingObject, T>::value>::type> int pushToLua(lua_State* L, T* obj)
{
    if (obj)
    {
        BindingObject* ptr = obj;
        return pushToLua(L, ptr);
    }
    lua_pushnil(L);
    return 1;
}

template<typename T> int pushToLua(lua_State* L, Vector2<T> f)
{
    lua_newtable(L);
    luaL_getmetatable(L, "vector2");
    lua_setmetatable(L, -2);
    lua_pushnumber(L, f.x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, f.y);
    lua_setfield(L, -2, "y");
    return 1;
}

template<typename T> int pushToLua(lua_State* L, Vector3<T> f)
{
    lua_newtable(L);
    luaL_getmetatable(L, "vector3");
    lua_setmetatable(L, -2);
    lua_pushnumber(L, f.x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, f.y);
    lua_setfield(L, -2, "y");
    lua_pushnumber(L, f.z);
    lua_setfield(L, -2, "z");
    return 1;
}

template<class TYPE, typename RET> class callClassHelper
{
public:
    template<typename... ARGS, std::size_t... N> static int doCall(lua_State* L, TYPE* obj, RET(TYPE::*f)(ARGS...), std::tuple<remove_cvref<ARGS>...>& args, sequence<N...>)
    {
        return pushToLua(L, (obj->*(f))(std::get<N>(args)...));
    }
    template<typename... ARGS, std::size_t... N> static int doCall(lua_State* L, TYPE* obj, RET(TYPE::*f)(ARGS...) const, std::tuple<remove_cvref<ARGS>...>& args, sequence<N...>)
    {
        return pushToLua(L, (obj->*(f))(std::get<N>(args)...));
    }
};

template<class TYPE> class callClassHelper<TYPE, void>
{
public:
    template<typename... ARGS, std::size_t... N> static int doCall(lua_State* L, TYPE* obj, void(TYPE::*f)(ARGS...), std::tuple<remove_cvref<ARGS>...>& args, sequence<N...>)
    {
        (obj->*(f))(std::get<N>(args)...);
        return pushToLua(L, obj);
    }
    template<typename... ARGS, std::size_t... N> static int doCall(lua_State* L, TYPE* obj, void(TYPE::*f)(ARGS...) const, std::tuple<remove_cvref<ARGS>...>& args, sequence<N...>)
    {
        (obj->*(f))(std::get<N>(args)...);
        return pushToLua(L, obj);
    }
};

template<typename RET> class callFunctionHelper
{
public:
    template<typename... ARGS, std::size_t... N> static int doCall(lua_State* L, RET(*f)(ARGS...), std::tuple<remove_cvref<ARGS>...>& args, sequence<N...>)
    {
        return pushToLua(L, (*f)(std::get<N>(args)...));
    }
};
template<> class callFunctionHelper<void>
{
public:
    template<typename... ARGS, std::size_t... N> static int doCall(lua_State* L, void(*f)(ARGS...), std::tuple<remove_cvref<ARGS>...>& args, sequence<N...>)
    {
        (*f)(std::get<N>(args)...);
        return 0;
    }
};

template<class TYPE, typename RET, typename... ARGS> int callMember(lua_State* L)
{
    int result;
    {
        typedef RET(TYPE::*FT)(ARGS...);
        FT* f = reinterpret_cast<FT*>(lua_touserdata(L, lua_upvalueindex(1)));
        TYPE* obj = convertFromLua(L, typeIdentifier<TYPE*>{}, lua_upvalueindex(2));
        if (!obj)
            return 0;
        std::tuple<remove_cvref<ARGS>...> args = getArgs<remove_cvref<ARGS>...>(L);
        result = callClassHelper<TYPE, RET>::doCall(L, obj, *f, args, typename sequenceGenerator<sizeof...(ARGS)>::type());
    }
    if constexpr (std::is_same_v<RET, Yield>)
        return lua_yield(L, result);
    return result;
}

template<class TYPE> int callMemberLua(lua_State* L)
{
    typedef int(TYPE::*FT)(lua_State*);
    FT* f = reinterpret_cast<FT*>(lua_touserdata(L, lua_upvalueindex(1)));
    TYPE* obj = convertFromLua(L, typeIdentifier<TYPE*>{}, lua_upvalueindex(2));
    if (!obj)
        return 0;
    return (obj->*(*f))(L);
}

template<class TYPE, typename RET, typename... ARGS> int callConstMember(lua_State* L)
{
    int result;
    {
        typedef RET(TYPE::*FT)(ARGS...) const;
        FT* f = reinterpret_cast<FT*>(lua_touserdata(L, lua_upvalueindex(1)));
        TYPE* obj = convertFromLua(L, typeIdentifier<TYPE*>{}, lua_upvalueindex(2));
        if (!obj)
            return 0;
        std::tuple<remove_cvref<ARGS>...> args = getArgs<remove_cvref<ARGS>...>(L);
        result = callClassHelper<TYPE, RET>::doCall(L, obj, *f, args, typename sequenceGenerator<sizeof...(ARGS)>::type());
    }
    if constexpr (std::is_same_v<RET, Yield>)
        return lua_yield(L, result);
    return result;
}

template<typename RET, typename... ARGS> int callFunction(lua_State* L)
{
    int result;
    {
        typedef RET(*FT)(ARGS...);
        FT* f = reinterpret_cast<FT*>(lua_touserdata(L, lua_upvalueindex(1)));
        std::tuple<remove_cvref<ARGS>...> args = getArgs<remove_cvref<ARGS>...>(L);
        result = callFunctionHelper<RET>::doCall(L, *f, args, typename sequenceGenerator<sizeof...(ARGS)>::type());
    }
    if constexpr (std::is_same_v<RET, Yield>)
        return lua_yield(L, result);
    return result;
}

template<typename TYPE> int getProperty(lua_State* L)
{
    TYPE* t = reinterpret_cast<TYPE*>(lua_touserdata(L, lua_upvalueindex(1)));
    return pushToLua(L, *t);
}

template<typename TYPE> int setProperty(lua_State* L)
{
    TYPE* t = reinterpret_cast<TYPE*>(lua_touserdata(L, lua_upvalueindex(1)));
    *t = convertFromLua(L, typeIdentifier<TYPE>{}, lua_absindex(L, -1));
    return 0;
}

template<typename T, class = typename std::enable_if<std::is_base_of<BindingObject, T>::value>::type> T* convertFromLua(lua_State* L, typeIdentifier<T*>, int index)
{
    luaL_checktype(L, index, LUA_TTABLE);
    lua_getmetatable(L, index);
    lua_getfield(L, -1, "object_ptr");
    luaL_checktype(L, -1, LUA_TLIGHTUSERDATA);

    T* obj = static_cast<T*>(static_cast<BindingObject*>(lua_touserdata(L, -1)));
    lua_pop(L, 2);
    return obj;
}

template<typename T, class = typename std::enable_if<std::is_base_of<BindingObject, T>::value>::type> sp::P<T> convertFromLua(lua_State* L, typeIdentifier<sp::P<T>>, int index)
{
    luaL_checktype(L, index, LUA_TTABLE);
    lua_getmetatable(L, index);
    lua_getfield(L, -1, "object_ptr");
    luaL_checktype(L, -1, LUA_TLIGHTUSERDATA);

    sp::P<T> obj = sp::P<BindingObject>(static_cast<BindingObject*>(lua_touserdata(L, -1)));
    lua_pop(L, 2);
    return obj;
}

static inline lua_State* convertFromLua(lua_State* L, typeIdentifier<lua_State*>, int index)
{
    return L;
}

static inline bool convertFromLua(lua_State* L, typeIdentifier<bool>, int index)
{
    return lua_toboolean(L, index);
}

static inline int convertFromLua(lua_State* L, typeIdentifier<int>, int index)
{
    return luaL_checkinteger(L, index);
}

static inline float convertFromLua(lua_State* L, typeIdentifier<float>, int index)
{
    return luaL_checknumber(L, index);
}

static inline double convertFromLua(lua_State* L, typeIdentifier<double>, int index)
{
    return luaL_checknumber(L, index);
}

static inline string convertFromLua(lua_State* L, typeIdentifier<string>, int index)
{
    return luaL_checkstring(L, index);
}

template<typename T> Vector2<T> convertFromLua(lua_State* L, typeIdentifier<Vector2<T>>, int index)
{
    luaL_checktype(L, index, LUA_TTABLE);
    lua_getfield(L, index, "x");
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        lua_geti(L, index, 1);
    }
    lua_getfield(L, index, "y");
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        lua_geti(L, index, 2);
    }
    T x = lua_tonumber(L, -2);
    T y = lua_tonumber(L, -1);
    lua_pop(L, 2);
    return Vector2<T>(x, y);
}

template<typename T> Vector3<T> convertFromLua(lua_State* L, typeIdentifier<Vector3<T>>, int index)
{
    luaL_checktype(L, index, LUA_TTABLE);
    lua_getfield(L, index, "x");
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        lua_geti(L, index, 1);
    }
    lua_getfield(L, index, "y");
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        lua_geti(L, index, 2);
    }
    lua_getfield(L, index, "z");
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        lua_geti(L, index, 3);
    }
    T x = lua_tonumber(L, -3);
    T y = lua_tonumber(L, -2);
    T z = lua_tonumber(L, -1);
    lua_pop(L, 3);
    return Vector3<T>(x, y, z);
}

static inline Variant convertFromLua(lua_State* L, typeIdentifier<Variant>, int index)
{
    if (lua_isnoneornil(L, index))
        return Variant();
    if (lua_isinteger(L, index))
        return int(lua_tointeger(L, index));
    if (lua_isnumber(L, index))
        return lua_tonumber(L, index);
    if (lua_isstring(L, index))
        return Variant(lua_tostring(L, index));
    return Variant();
}

}//namespace script
}//namespace sp

#endif//SP2_SCRIPT_LUA_BINDINGS_H
