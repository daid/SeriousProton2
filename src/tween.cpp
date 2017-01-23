#include <sp2/tween.h>

namespace sp {

template<> Color Tween<Color>::tweenApply(float f, const Color& value0, const Color& value1)
{
    return Color(
        value0.r + (value1.r - value0.r) * f,
        value0.g + (value1.g - value0.g) * f,
        value0.b + (value1.b - value0.b) * f,
        value0.a + (value1.a - value0.a) * f
    );
}

};//!namespace sp
