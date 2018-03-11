#ifndef SP2_NON_COPYABLE_H
#define SP2_NON_COPYABLE_H

class NonCopyable
{
protected:
    NonCopyable() = default;
private:
    NonCopyable(const NonCopyable&) = delete;
    void operator=(const NonCopyable&) = delete;
};

#endif//SP2_NON_COPYABLE_H
