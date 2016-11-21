#ifndef SP2_MATH_VECTOR_H
#define SP2_MATH_VECTOR_H

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>
#include <cmath>
#include <ostream>

namespace sp {

constexpr double pi = 3.14159265358979323846;

template <typename T> T length(const sf::Vector2<T>& v)
{
    return sqrtf(v.x*v.x+v.y*v.y);
}

template <typename T> sf::Vector2<T> normalize(const sf::Vector2<T>& v)
{
    return v / length(v);
}

typedef sf::Vector2<float> Vector2f;
typedef sf::Vector2<double> Vector2d;

template <typename T> static inline std::ostream& operator<<(std::ostream& stream, const sf::Vector2<T> v)
{
    stream << v.x << "," << v.y;
    return stream;
}
};//!namespace sp

namespace sp {

template <typename T> T length(const sf::Vector3<T>& v)
{
    return sqrtf(v.x*v.x+v.y*v.y+v.z*v.z);
}

template <typename T> sf::Vector3<T> normalize(const sf::Vector3<T>& v)
{
    return v / length(v);
}

typedef sf::Vector3<float> Vector3f;
typedef sf::Vector3<double> Vector3d;

template <typename T> static inline std::ostream& operator<<(std::ostream& stream, const sf::Vector3<T> v)
{
    stream << v.x << "," << v.y << "," << v.z;
    return stream;
}

/* Return the difference between angle_a and angle_b within a range of -180 and 180 degrees */
template <typename T> T angleDifference(const T& angle_a, const T& angle_b)
{
    T ret = (angle_b - angle_a);
    while(ret > 180) ret -= 360;
    while(ret < -180) ret += 360;
    return ret;
}

};//!namespace sp

#endif//SP2_MATH_VECTOR_H
