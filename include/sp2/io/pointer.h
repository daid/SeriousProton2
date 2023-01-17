#ifndef SP2_IO_POINTER_H
#define SP2_IO_POINTER_H

#include <sp2/string.h>

namespace sp {
namespace io {

class Pointer
{
public:
    enum class Button
    {
        Touch,
        Left,
        Middle,
        Right,
        X1,
        X2,
        Unknown
    };

    enum class Wheel
    {
        Up,
        Down,
        Left,
        Right,
    };
private:
};

}//namespace io
}//namespace sp

#endif//CLIPBOARD_H
