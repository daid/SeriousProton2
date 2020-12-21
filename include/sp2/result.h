#ifndef SP2_RESULT_H
#define SP2_RESULT_H

#include <sp2/logging.h>

namespace sp {

template<typename T> class Result
{
public:
    Result(T&& value)
    : success(true), ok_value(std::forward<T>(value))
    {
    }

    bool isOk()
    {
        return success;
    }

    bool isErr()
    {
        return !success;
    }

    const T& value()
    {
        if (!success)
        {
            LOG(Error, err_value);
            success = true;
        }
        return ok_value;
    }

    const sp::string& error()
    {
        return err_value;
    }

    static Result<T> makeError(sp::string&& error)
    {
        return Result(std::forward<sp::string>(error), {});
    }
private:
    class ErrorConstructor{};

    Result(sp::string&& error, const ErrorConstructor&)
    : success(false), err_value(std::forward<sp::string>(error))
    {
    }

    bool success;
    T ok_value;
    sp::string err_value;
};

}

#endif//SP2_RESULT_H
