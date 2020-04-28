#ifndef SP2_SCRIPT_COROUTINE_H
#define SP2_SCRIPT_COROUTINE_H

#include <memory>
#include <sp2/script/luaBindings.h>
#include <sp2/logging.h>

namespace sp {
namespace script {

class Coroutine : NonCopyable
{
public:
    Coroutine(lua_State* origin, lua_State* L);
    ~Coroutine();

    /** Resume the coroutine.
     *  Resumes the function that was yielded.
     *  Returns true if the coroutine was still yielded.
     *  Returns false if the coroutine has ended.
     */
    template<typename... ARGS> bool resume(ARGS... args)
    {
        last_error = "";
        if (!L)
            return false;
        //Set the hook as it was already, so the internal counter gets reset for sandboxed environments.
        lua_sethook(L, lua_gethook(L), lua_gethookmask(L), lua_gethookcount(L));

        int arg_count = pushArgs(L, args...);
        int status = lua_resume(L, nullptr, arg_count);

        if (status == LUA_YIELD)
            return true;
        if (status != LUA_OK)
        {
            last_error = lua_tostring(L, -1);
            LOG(Error, "Coroutine resume error:", last_error);
        }
        release();
        return false;
    }

    const string& getLastError()
    {
        return last_error;
    }

    //Get the current yielded location in the source code. Or -1 if that is not available, or the coroutine is no longer yielded.
    int getCurrentLineNumber();
    string getCurrentSource();
private:
    void release();

    int pushArgs(lua_State* L)
    {
        return 0;
    }

    template<typename ARG, typename... ARGS> int pushArgs(lua_State* L, ARG arg, ARGS... args)
    {
        pushToLua(L, arg);
        return 1 + pushArgs(L, args...);
    }

    lua_State* L;
    string last_error;

    friend class Environment;
};

typedef std::shared_ptr<Coroutine> CoroutinePtr;

}//namespace script
}//namespace sp

#endif//SP2_SCRIPT_COROUTINE_H
