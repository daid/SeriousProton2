#ifndef SP2_MATH_QUATERNION_H
#define SP2_MATH_QUATERNION_H

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>
#include <cmath>

#include <sp2/math/vector.h>

namespace sp {

template<typename T> class Quaternion
{
public:
    T x, y, z, w;
    
    Quaternion()
    : x(0), y(0), z(0), w(1)
    {}
    
    Quaternion(T x, T y, T z, T w)
    : x(x), y(y), z(z), w(w)
    {}
    
    Quaternion operator*(const Quaternion& q) const
    {
        return Quaternion(
            x * q.w + q.x * w + (y * q.z - q.y * z),
            y * q.w + q.y * w + (z * q.x - q.z * x),
            z * q.w + q.z * w + (x * q.y - q.x * y),
            q.w * w - (x*q.x + y*q.y + z*q.z)
        );
    }
    
    T length()
    {
        return std::sqrt(x * x + y * y + z * z + w * w);
    }
    
    void normalize()
    {
        T len = std::sqrt(x * x + y * y + z * z + w * w);
        x /= len;
        y /= len;
        z /= len;
        w /= len;
    }
    
    static Quaternion fromAxisAngle(sf::Vector3<T> axis, T angle)
    {
        sf::Vector3<T> a = sp::normalize(axis);
        T half_angle = (angle / 2.0) / 180.0 * pi;
        T c = std::cos(half_angle);
        T s = std::sin(half_angle);
        Quaternion q(a.x * s, a.y * s, a.z * s, c);
        q.normalize();
        return q;
    }

    static Quaternion fromAngle(T angle)
    {
        T half_angle = (angle / 2.0) / 180.0 * pi;
        T c = std::cos(half_angle);
        T s = std::sin(half_angle);
        Quaternion q(0, 0, s, c);
        q.normalize();
        return q;
    }
};

typedef Quaternion<float> Quaternionf;
typedef Quaternion<double> Quaterniond;

};//!namespace sp

#endif//SP2_MATH_QUATERNION_H
