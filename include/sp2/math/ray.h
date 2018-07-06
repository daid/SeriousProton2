#ifndef SP2_MATH_RAY_H
#define SP2_MATH_RAY_H

#include <cmath>
#include <sp2/math/vector.h>

namespace sp {

template<typename T> class Ray
{
public:
    Ray() {}
    Ray(T start, T end) : start(start), end(end) {}
    
    T start;
    T end;
};

typedef Ray<Vector2f> Ray2f;
typedef Ray<Vector2d> Ray2d;
typedef Ray<Vector3f> Ray3f;
typedef Ray<Vector3d> Ray3d;

template <typename T> static inline std::ostream& operator<<(std::ostream& stream, const Ray<T>& r)
{
    stream << r.start << "->" << r.end;
    return stream;
}

};//namespace sp

#endif//SP2_MATH_RAY_H
