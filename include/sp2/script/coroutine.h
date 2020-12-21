#ifndef SP2_SCRIPT_COROUTINE_H
#define SP2_SCRIPT_COROUTINE_H

#include <memory>
#include <sp2/script/luaState.h>

namespace sp {
namespace script {

class Coroutine : public LuaState
{
public:
    Coroutine(lua_State* origin, lua_State* L);
    ~Coroutine();

    /** Resume the coroutine.
     *  Resumes the function that was yielded.
     *  Returns true if the coroutine was still yielded.
     *  Returns false if the coroutine has ended.
     */
    template<typename... ARGS> Result<bool> resume(ARGS... args)
    {
        if (!lua)
            return false;

        int arg_count = pushArgs(lua, args...);
        auto result = resumeInternal(arg_count);
        if (result.isErr() || !result.value())
            release();
        return result;
    }

    //Get the current yielded location in the source code. Or -1 if that is not available, or the coroutine is no longer yielded.
    int getCurrentLineNumber();
    string getCurrentSource();
private:
    void release();
};

typedef std::shared_ptr<Coroutine> CoroutinePtr;

}//namespace script
}//namespace sp

#endif//SP2_SCRIPT_COROUTINE_H
