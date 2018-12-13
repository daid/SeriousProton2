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

template<> Vector2d Tween<Vector2d>::tweenApply(float f, const Vector2d& value0, const Vector2d& value1)
{
    return Vector2d(
        value0.x + (value1.x - value0.x) * f,
        value0.y + (value1.y - value0.y) * f
    );
}

template<> Vector3d Tween<Vector3d>::tweenApply(float f, const Vector3d& value0, const Vector3d& value1)
{
    return Vector3d(
        value0.x + (value1.x - value0.x) * f,
        value0.y + (value1.y - value0.y) * f,
        value0.z + (value1.z - value0.z) * f
    );
}

};//namespace sp
