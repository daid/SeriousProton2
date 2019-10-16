#ifndef SP2_MATH_VECTOR2_H
#define SP2_MATH_VECTOR2_H

#include <cmath>
#include <ostream>

#include <sp2/math/vector.h>

namespace sp {

template<typename T> class Vector2
{
public:
    T x, y;
    
    inline Vector2()
    : x(0), y(0)
    {
    }
    
    inline Vector2(T x, T y)
    : x(x), y(y)
    {
    }
    
    template <typename T2> explicit inline Vector2(const Vector2<T2>& v)
    : x(v.x), y(v.y)
    {
    }

    inline T length()
    {
        return std::sqrt(x * x + y * y);
    }
    
    inline Vector2<T> normalized()
    {
        T f = length();
        if (f < 0.000001)
            return Vector2<T>(1, 0);
        return *this / f;
    }
    
    inline T cross(const Vector2<T>& v)
    {
        return x * v.y - y * v.x;
    }

    inline T dot(const Vector2<T>& v)
    {
        return x * v.x + y * v.y;
    }

    T angle()
    {
        return std::atan2(y, x) / pi * 180.0;
    }

    inline Vector2<T> rotate(T angle)
    {
        angle = angle / 180.0 * pi;
        T s = std::sin(angle);
        T c = std::cos(angle);
        return Vector2<T>(c * x - s * y, s * x + c * y);
    }
};

template <typename T> static inline Vector2<T> operator -(const Vector2<T>& a) { return Vector2<T>(-a.x, -a.y); }
template <typename T> static inline Vector2<T> operator +(const Vector2<T>& a, const Vector2<T>& b) { return Vector2<T>(a.x+b.x, a.y+b.y); }
template <typename T> static inline Vector2<T> operator -(const Vector2<T>& a, const Vector2<T>& b) { return Vector2<T>(a.x-b.x, a.y-b.y); }
template <typename T> static inline Vector2<T> operator *(const Vector2<T>& a, const T b) { return Vector2<T>(a.x*b, a.y*b); }
template <typename T> static inline Vector2<T> operator /(const Vector2<T>& a, const T b) { return Vector2<T>(a.x/b, a.y/b); }
template <typename T> static inline void operator +=(Vector2<T>& a, const Vector2<T>& b) { a.x += b.x; a.y += b.y; }
template <typename T> static inline void operator -=(Vector2<T>& a, const Vector2<T>& b) { a.x -= b.x; a.y -= b.y; }
template <typename T> static inline void operator *=(Vector2<T>& a, const T b) { a.x *= b; a.y *= b; }
template <typename T> static inline void operator /=(Vector2<T>& a, const T b) { a.x /= b; a.y /= b; }
template <typename T> static inline bool operator ==(const Vector2<T>& a, const Vector2<T>& b) { return a.x == b.x && a.y == b.y; }
template <typename T> static inline bool operator !=(const Vector2<T>& a, const Vector2<T>& b) { return a.x != b.x || a.y != b.y; }

typedef Vector2<float> Vector2f;
typedef Vector2<double> Vector2d;
typedef Vector2<int> Vector2i;

template <typename T> static inline std::ostream& operator<<(std::ostream& stream, const Vector2<T> v)
{
    stream << v.x << "," << v.y;
    return stream;
}

}//namespace sp

namespace std
{
    template<typename T> struct hash<sp::Vector2<T> >
    {
        std::size_t operator()(const sp::Vector2<T>& v) const
        {
            return int(v.x) ^ (int(v.y) << 8);
        }
    };
}

#endif//SP2_MATH_VECTOR_H
