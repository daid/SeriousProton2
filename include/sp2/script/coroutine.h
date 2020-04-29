#ifndef SP2_SCRIPT_COROUTINE_H
#define SP2_SCRIPT_COROUTINE_H

#include <memory>
#include <sp2/script/luaBindings.h>
#include <sp2/logging.h>

namespace sp {
namespace script {

class Environment;
class Coroutine : NonCopyable
{
public:
    Coroutine(P<Environment> environment, lua_State* origin, lua_State* L);
    ~Coroutine();

    /** Resume the coroutine.
     *  Resumes the function that was yielded.
     *  Returns true if the coroutine was still yielded.
     *  Returns false if the coroutine has ended.
     */
    template<typename... ARGS> bool resume(ARGS... args)
    {
        last_error = "";
        if (!environment || !L)
            return false;

        int arg_count = pushArgs(L, args...);
        return internalResume(arg_count);
    }

    const string& getLastError()
    {
        return last_error;
    }

    //Get the current yielded location in the source code. Or -1 if that is not available, or the coroutine is no longer yielded.
    int getCurrentLineNumber();
    string getCurrentSource();
private:
    bool internalResume(int arg_count);
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
    P<Environment> environment;
};

typedef std::shared_ptr<Coroutine> CoroutinePtr;

}//namespace script
}//namespace sp

#endif//SP2_SCRIPT_COROUTINE_H
