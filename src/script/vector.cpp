#include <sp2/script/luaBindings.h>
#include <sp2/logging.h>
#include <sp2/assert.h>

namespace sp {
namespace script {

static int vector2Create(lua_State* L)
{
    double x = luaL_checknumber(L, 1);
    double y = luaL_checknumber(L, 2);
    return pushToLua(L, Vector2d(x, y));
}

static int vector2Add(lua_State* L)
{
    Vector2<lua_Number> v0 = convertFromLua(L, typeIdentifier<Vector2<lua_Number>>{}, 1);
    Vector2<lua_Number> v1 = convertFromLua(L, typeIdentifier<Vector2<lua_Number>>{}, 2);
    return pushToLua(L, v0 + v1);
}

static int vector2Sub(lua_State* L)
{
    Vector2<lua_Number> v0 = convertFromLua(L, typeIdentifier<Vector2<lua_Number>>{}, 1);
    Vector2<lua_Number> v1 = convertFromLua(L, typeIdentifier<Vector2<lua_Number>>{}, 2);
    return pushToLua(L, v0 - v1);
}

static int vector2Mul(lua_State* L)
{
    Vector2<lua_Number> v0 = convertFromLua(L, typeIdentifier<Vector2<lua_Number>>{}, 1);
    lua_Number v1 = convertFromLua(L, typeIdentifier<lua_Number>{}, 2);
    return pushToLua(L, v0 * v1);
}

static int vector2Div(lua_State* L)
{
    Vector2<lua_Number> v0 = convertFromLua(L, typeIdentifier<Vector2<lua_Number>>{}, 1);
    lua_Number v1 = convertFromLua(L, typeIdentifier<lua_Number>{}, 2);
    return pushToLua(L, v0 / v1);
}

static int vector2Unm(lua_State* L)
{
    Vector2<lua_Number> v0 = convertFromLua(L, typeIdentifier<Vector2<lua_Number>>{}, 1);
    return pushToLua(L, -v0);
}

static int vector2ToString(lua_State* L)
{
    Vector2<lua_Number> v0 = convertFromLua(L, typeIdentifier<Vector2<lua_Number>>{}, 1);
    lua_pushstring(L, ("{" + string(v0.x) + "," + string(v0.y) + "}").c_str());
    return 1;
}

static int vector2Length(lua_State* L)
{
    Vector2<lua_Number> v0 = convertFromLua(L, typeIdentifier<Vector2<lua_Number>>{}, 1);
    return pushToLua(L, v0.length());
}

static int vector2Normalized(lua_State* L)
{
    Vector2<lua_Number> v0 = convertFromLua(L, typeIdentifier<Vector2<lua_Number>>{}, 1);
    return pushToLua(L, v0.normalized());
}

static int vector2Dot(lua_State* L)
{
    Vector2<lua_Number> v0 = convertFromLua(L, typeIdentifier<Vector2<lua_Number>>{}, 1);
    Vector2<lua_Number> v1 = convertFromLua(L, typeIdentifier<Vector2<lua_Number>>{}, 2);
    return pushToLua(L, v0.dot(v1));
}

static int vector2Cross(lua_State* L)
{
    Vector2<lua_Number> v0 = convertFromLua(L, typeIdentifier<Vector2<lua_Number>>{}, 1);
    Vector2<lua_Number> v1 = convertFromLua(L, typeIdentifier<Vector2<lua_Number>>{}, 2);
    return pushToLua(L, v0.cross(v1));
}

static int vector2Angle(lua_State* L)
{
    Vector2<lua_Number> v0 = convertFromLua(L, typeIdentifier<Vector2<lua_Number>>{}, 1);
    return pushToLua(L, v0.angle());
}

static int vector2Rotate(lua_State* L)
{
    Vector2<lua_Number> v0 = convertFromLua(L, typeIdentifier<Vector2<lua_Number>>{}, 1);
    lua_Number v1 = convertFromLua(L, typeIdentifier<lua_Number>{}, 2);
    return pushToLua(L, v0.rotate(v1));
}

static luaL_Reg vector2_functions[] = {
    {"__add", vector2Add},
    {"__sub", vector2Sub},
    {"__mul", vector2Mul},
    {"__div", vector2Div},
    {"__unm", vector2Unm},
    {"__tostring", vector2ToString},
    {"__len", vector2Length},
    {"length", vector2Length},
    {"normalized", vector2Normalized},
    {"dot", vector2Dot},
    {"cross", vector2Cross},
    {"angle", vector2Angle},
    {"rotate", vector2Rotate},
    {nullptr, nullptr},
};

static int vector3Create(lua_State* L)
{
    double x = luaL_checknumber(L, 1);
    double y = luaL_checknumber(L, 2);
    double z = luaL_checknumber(L, 3);
    return pushToLua(L, Vector3d(x, y, z));
}

static int vector3Add(lua_State* L)
{
    Vector3<lua_Number> v0 = convertFromLua(L, typeIdentifier<Vector3<lua_Number>>{}, 1);
    Vector3<lua_Number> v1 = convertFromLua(L, typeIdentifier<Vector3<lua_Number>>{}, 2);
    return pushToLua(L, v0 + v1);
}

static int vector3Sub(lua_State* L)
{
    Vector3<lua_Number> v0 = convertFromLua(L, typeIdentifier<Vector3<lua_Number>>{}, 1);
    Vector3<lua_Number> v1 = convertFromLua(L, typeIdentifier<Vector3<lua_Number>>{}, 2);
    return pushToLua(L, v0 - v1);
}

static int vector3Mul(lua_State* L)
{
    Vector3<lua_Number> v0 = convertFromLua(L, typeIdentifier<Vector3<lua_Number>>{}, 1);
    lua_Number v1 = convertFromLua(L, typeIdentifier<lua_Number>{}, 2);
    return pushToLua(L, v0 * v1);
}

static int vector3Div(lua_State* L)
{
    Vector3<lua_Number> v0 = convertFromLua(L, typeIdentifier<Vector3<lua_Number>>{}, 1);
    lua_Number v1 = convertFromLua(L, typeIdentifier<lua_Number>{}, 2);
    return pushToLua(L, v0 / v1);
}

static int vector3Unm(lua_State* L)
{
    Vector3<lua_Number> v0 = convertFromLua(L, typeIdentifier<Vector3<lua_Number>>{}, 1);
    return pushToLua(L, -v0);
}

static int vector3ToString(lua_State* L)
{
    Vector3<lua_Number> v0 = convertFromLua(L, typeIdentifier<Vector3<lua_Number>>{}, 1);
    lua_pushstring(L, ("{" + string(v0.x) + "," + string(v0.y) + "," + string(v0.z) + "}").c_str());
    return 1;
}

static int vector3Length(lua_State* L)
{
    Vector3<lua_Number> v0 = convertFromLua(L, typeIdentifier<Vector3<lua_Number>>{}, 1);
    return pushToLua(L, v0.length());
}

static int vector3Normalized(lua_State* L)
{
    Vector3<lua_Number> v0 = convertFromLua(L, typeIdentifier<Vector3<lua_Number>>{}, 1);
    return pushToLua(L, v0.normalized());
}

static int vector3Dot(lua_State* L)
{
    Vector3<lua_Number> v0 = convertFromLua(L, typeIdentifier<Vector3<lua_Number>>{}, 1);
    Vector3<lua_Number> v1 = convertFromLua(L, typeIdentifier<Vector3<lua_Number>>{}, 2);
    return pushToLua(L, v0.dot(v1));
}

static int vector3Cross(lua_State* L)
{
    Vector3<lua_Number> v0 = convertFromLua(L, typeIdentifier<Vector3<lua_Number>>{}, 1);
    Vector3<lua_Number> v1 = convertFromLua(L, typeIdentifier<Vector3<lua_Number>>{}, 2);
    return pushToLua(L, v0.cross(v1));
}

static luaL_Reg vector3_functions[] = {
    {"__add", vector3Add},
    {"__sub", vector3Sub},
    {"__mul", vector3Mul},
    {"__div", vector3Div},
    {"__unm", vector3Unm},
    {"__tostring", vector3ToString},
    {"__len", vector3Length},
    {"length", vector3Length},
    {"normalized", vector3Normalized},
    {"dot", vector3Dot},
    {"cross", vector3Cross},
    //{"angle", vector2Angle},
    //{"rotate", vector2Rotate},
    {nullptr, nullptr},
};

void addVectorMetatables()
{
    luaL_newmetatable(global_lua_state, "vector2");
    lua_pushvalue(global_lua_state, -1);
    lua_setfield(global_lua_state, -2, "__index");
    luaL_setfuncs(global_lua_state, vector2_functions, 0);
    lua_pop(global_lua_state, 1);
    luaL_newmetatable(global_lua_state, "vector3");
    lua_pushvalue(global_lua_state, -1);
    lua_setfield(global_lua_state, -2, "__index");
    luaL_setfuncs(global_lua_state, vector3_functions, 0);
    lua_pop(global_lua_state, 1);
    
    lua_register(global_lua_state, "Vector2", vector2Create);
    lua_register(global_lua_state, "Vector3", vector3Create);
}

};//namespace script
};//namespace sp
