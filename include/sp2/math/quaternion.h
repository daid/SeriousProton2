#ifndef SP2_MATH_QUATERNION_H
#define SP2_MATH_QUATERNION_H

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
    
    template<typename VT> Vector2<VT> operator*(const Vector2<VT>& v) const
    {
        T vMult = 2.0 * (x * v.x + y * v.y);
        T crossMult = 2.0 * w;
        T pMult = crossMult * w - 1.0;

        return Vector2<VT>(
            pMult * v.x + vMult * x + crossMult * (- z * v.y),
            pMult * v.y + vMult * y + crossMult * (z * v.x));
    }

    template<typename VT> Vector3<VT> operator*(const Vector3<VT>& v) const
    {
        T vMult = 2.0 * (x * v.x + y * v.y + z * v.z);
        T crossMult = 2.0 * w;
        T pMult = crossMult * w - 1.0;

        return Vector3<VT>(
            pMult * v.x + vMult * x + crossMult * (y * v.z - z * v.y),
            pMult * v.y + vMult * y + crossMult * (z * v.x - x * v.z),
            pMult * v.z + vMult * z + crossMult * (x * v.y - y * v.x));
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
    
    static Quaternion fromAxisAngle(Vector3<T> axis, T angle)
    {
        Vector3<T> a = axis.normalized();
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

template <typename T> static inline std::ostream& operator<<(std::ostream& stream, const Quaternion<T> q)
{
    stream << q.x << "," << q.y << "," << q.z << "," << q.w;
    return stream;
}

};//!namespace sp

#endif//SP2_MATH_QUATERNION_H
