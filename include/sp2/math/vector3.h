#ifndef SP2_MATH_VECTOR3_H
#define SP2_MATH_VECTOR3_H

#include <cmath>
#include <ostream>

namespace sp {

template<typename T> class Vector3
{
public:
    T x, y, z;
    
    inline Vector3()
    : x(0), y(0), z(0)
    {
    }
    
    inline Vector3(T x, T y, T z)
    : x(x), y(y), z(z)
    {
    }
    
    template <typename T2> explicit inline Vector3(const Vector3<T2>& v)
    : x(v.x), y(v.y), z(v.z)
    {
    }

    inline T length()
    {
        return std::sqrt(x * x + y * y + z * z);
    }
    
    inline Vector3<T> normalized()
    {
        return *this / length();
    }
    
    inline T dot(const Vector3<T>& v)
    {
        return x * v.x + y * v.y + z * v.z;
    }
};

template <typename T> static inline Vector3<T> operator -(const Vector3<T>& a) { return Vector3<T>(-a.x, -a.y, -a.z); }
template <typename T> static inline Vector3<T> operator +(const Vector3<T>& a, const Vector3<T>& b) { return Vector3<T>(a.x+b.x, a.y+b.y, a.z+b.z); }
template <typename T> static inline Vector3<T> operator -(const Vector3<T>& a, const Vector3<T>& b) { return Vector3<T>(a.x-b.x, a.y-b.y, a.z-b.z); }
template <typename T> static inline Vector3<T> operator *(const Vector3<T>& a, const T b) { return Vector3<T>(a.x*b, a.y*b, a.z*b); }
template <typename T> static inline Vector3<T> operator /(const Vector3<T>& a, const T b) { return Vector3<T>(a.x/b, a.y/b, a.z/b); }
template <typename T> static inline void operator +=(Vector3<T>& a, const Vector3<T>& b) { a.x += b.x; a.y += b.y; a.z += b.z; }
template <typename T> static inline void operator -=(Vector3<T>& a, const Vector3<T>& b) { a.x -= b.x; a.y -= b.y; a.z -= b.z; }
template <typename T> static inline void operator *=(Vector3<T>& a, const T b) { a.x *= b; a.y *= b; a.z *= b; }
template <typename T> static inline void operator /=(Vector3<T>& a, const T b) { a.x /= b; a.y /= b; a.z /= b; }
template <typename T> static inline bool operator ==(const Vector3<T>& a, const Vector3<T>& b) { return a.x == b.x && a.y == b.y && a.z == b.z; }
template <typename T> static inline bool operator !=(const Vector3<T>& a, const Vector3<T>& b) { return a.x != b.x || a.y != b.y || a.z != b.z; }

typedef Vector3<float> Vector3f;
typedef Vector3<double> Vector3d;
typedef Vector3<int> Vector3i;

template <typename T> static inline std::ostream& operator<<(std::ostream& stream, const Vector3<T> v)
{
    stream << v.x << "," << v.y << "," << v.z;
    return stream;
}

};//namespace sp

namespace std
{
    template<typename T> struct hash<sp::Vector3<T> >
    {
        std::size_t operator()(const sp::Vector3<T>& v) const
        {
            return int(v.x) ^ (int(v.y) << 8) ^ (int(v.z) << 12);
        }
    };
}

#endif//SP2_MATH_VECTOR_H
