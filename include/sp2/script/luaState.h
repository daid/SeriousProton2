#ifndef SP2_SCRIPT_LUASTATE_H
#define SP2_SCRIPT_LUASTATE_H

#include <sp2/string.h>
#include <sp2/nonCopyable.h>
#include <memory>
#include <lua/lua.h>

namespace sp {
namespace script {

class Environment;
class Coroutine;
class LuaState : public NonCopyable
{
public:
    const string& getLastError() { return last_error; }

protected:
    int pushArgs(lua_State* L)
    {
        return 0;
    }

    template<typename ARG, typename... ARGS> int pushArgs(lua_State* L, ARG arg, ARGS... args)
    {
        pushToLua(L, arg);
        return 1 + pushArgs(L, args...);
    }

    //Execute a call, only thing on the stack should be the function and the arguments.
    bool callInternal(int arg_count);
    //Execute a resume, only thing on the stack should be the function and the arguments.
    bool resumeInternal(int arg_count);
    std::shared_ptr<Coroutine> callCoroutineInternal(lua_State* L, int arg_count);

    lua_State* lua = nullptr;

    void setLastError(string error);
private:
    string last_error;
};

}//namespace script
}//namespace sp

#endif//SP2_SCRIPT_ENVIRONMENT_H
