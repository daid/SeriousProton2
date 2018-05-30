#ifndef SP2_MATH_VECTOR_H
#define SP2_MATH_VECTOR_H

#include <cmath>
#include <ostream>

namespace sp {

constexpr double pi = 3.14159265358979323846;

/* Return the difference between angle_a and angle_b within a range of -180 and 180 degrees */
template <typename T> T angleDifference(const T& angle_a, const T& angle_b)
{
    T ret = (angle_b - angle_a);
    while(ret > 180) ret -= 360;
    while(ret < -180) ret += 360;
    return ret;
}

};//namespace sp

#include "vector2.h"
#include "vector3.h"

#endif//SP2_MATH_VECTOR_H
