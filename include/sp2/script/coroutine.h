#ifndef SP2_SCRIPT_COROUTINE_H
#define SP2_SCRIPT_COROUTINE_H

#include <memory>
#include <sp2/script/luaBindings.h>
#include <sp2/logging.h>

namespace sp {
namespace script {

class Coroutine
{
public:
    Coroutine(lua_State* L);
    ~Coroutine();

    /** Resume the coroutine.
     *  Resumes the function that was yielded.
     *  Returns true if the coroutine was still yielded.
     *  Returns false if the coroutine has ended.
     */
    template<typename... ARGS> bool resume(ARGS... args)
    {
        if (!L)
            return false;

        int arg_count = pushArgs(L, args...);
        int status = lua_resume(L, nullptr, arg_count);
        
        if (status == LUA_YIELD)
            return true;
        if (status != LUA_OK)
        {
            LOG(Error, "Coroutine resume error:", lua_tostring(L, -1));
        }
        release();
        return false;
    }

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
    
    friend class Environment;
};

typedef std::shared_ptr<Coroutine> CoroutinePtr;

};//namespace script
};//namespace sp

#endif//SP2_SCRIPT_COROUTINE_H
