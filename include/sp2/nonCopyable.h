#ifndef SP2_NON_COPYABLE_H
#define SP2_NON_COPYABLE_H

namespace sp {

class NonCopyable
{
protected:
    NonCopyable() = default;
private:
    NonCopyable(const NonCopyable&) = delete;
    void operator=(const NonCopyable&) = delete;
};

};//!namespace sp

#endif//SP2_NON_COPYABLE_H
