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

    template <typename T2> explicit inline Quaternion(const Quaternion<T2>& q)
    : x(q.x), y(q.y), z(q.z), w(q.w)
    {
    }
    
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
    
    Quaternion<T> normalized()
    {
        T len = std::sqrt(x * x + y * y + z * z + w * w);
        return Quaternion<T>(x / len, y / len, z / len, w / len);
    }
    
    Quaternion<T> inverse()
    {
        return Quaternion<T>(-x, -y, -z, w);
    }
    
    Quaternion<T> diff(const Quaternion<T>& other)
    {
        return inverse() * other;
    }
    
    Vector3<T> toAxisAngle()
    {
        T wclamped = std::max(-1.0, std::min(1.0, w));
        T angle = std::acos(wclamped) * 2.0 / sp::pi * 180.0;
        T d = std::sqrt(1-wclamped*wclamped);
        if (d < 0.0001)
            return Vector3<T>(0, 0, 0);
        if (angle > 180.0)
            angle = angle - 360.0;
        return Vector3<T>(x / d, y / d, z / d) * angle;
    }
    
    void normalize()
    {
        T len = std::sqrt(x * x + y * y + z * z + w * w);
        x /= len;
        y /= len;
        z /= len;
        w /= len;
    }
    
    Quaternion slerp(const Quaternion<T>& other, T t)
    {
        Quaternion<T> v = other;
        v.normalize();
        T dot = x * other.x + y * other.y + z * other.z + w * other.w;
        if (dot < 0)
        {
            v.x = -v.x;
            v.y = -v.y;
            v.z = -v.z;
            v.w = -v.w;
            dot = -dot;
        }
        if (dot > 0.9999)
        {
            Quaternion<T> result(x + (v.x - x) * t, y + (v.y - y) * t, z + (v.z - z) * t, w + (v.w - w) * t);
            result.normalize();
            return result;
        }
        
        T theta_0 = std::acos(dot);
        T theta = theta_0 * t;
        T sin_theta = std::sin(theta);
        T sin_theta_0 = std::sin(theta_0);

        T s0 = std::cos(theta) - dot * sin_theta / sin_theta_0;
        T s1 = sin_theta / sin_theta_0;

        return Quaternion<T>(x * s0 + v.x * s1, y * s0 + v.y * s1, z * s0 + v.z * s1, w * s0 + v.w * s1);
    }
    
    Quaternion conjugate()
    {
        return Quaternion<T>(-x, -y, -z, w);
    }
    
    static Quaternion fromAxisAngle(const Vector3<T>& axis, T angle)
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
    
    static Quaternion fromVectorToVector(const Vector3<T> v0, const Vector3<T>& v1)
    {
        Vector3<T> a = v0.cross(v1);
        return Quaternion<T>(a.x, a.y, a.z, std::sqrt(v0.dot(v0) + v1.dot(v1)) + v0.dot(v1));
    }
};

typedef Quaternion<float> Quaternionf;
typedef Quaternion<double> Quaterniond;

template <typename T> static inline std::ostream& operator<<(std::ostream& stream, const Quaternion<T> q)
{
    stream << q.x << "," << q.y << "," << q.z << "," << q.w;
    return stream;
}

}//namespace sp

#endif//SP2_MATH_QUATERNION_H
