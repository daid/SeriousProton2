#ifndef SP2_TWEAK_H
#define SP2_TWEAK_H

#include <sp2/string.h>
#include <sp2/math/vector.h>
#include <sp2/graphics/color.h>


namespace sp {

int tweak(string name, int value, int min, int max);
double tweak(string name, double value, double min, double max);
Color tweak(string name, Color color);
template<typename T> Vector2<T> tweak(string name, Vector2<T> value, T min, T max)
{
    value.x = tweak(name+".x", value.x, min, max);
    value.y = tweak(name+".y", value.y, min, max);
    return value;
}
template<typename T> Vector3<T> tweak(string name, Vector3<T> value, T min, T max)
{
    value.x = tweak(name+".x", value.x, min, max);
    value.y = tweak(name+".y", value.y, min, max);
    value.z = tweak(name+".z", value.z, min, max);
    return value;
}

};//namespace sp

#endif//SP2_TWEAK_H
