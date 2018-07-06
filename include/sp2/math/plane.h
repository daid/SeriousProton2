#ifndef SP2_MATH_PLANE_H
#define SP2_MATH_PLANE_H

#include <cmath>
#include <sp2/math/vector.h>

namespace sp {

template<typename T> class Plane
{
public:
    Plane() {}
    Plane(Vector3<T> normal, T distance) : normal(normal), distance(distance) {}
    
    Vector3<T> intersect(Ray<Vector3<T>> ray)
    {
        Vector3<T> w = ray.start - (normal * distance);
        Vector3<T> d = (ray.end - ray.start).normalized();

        T dot_d = normal.dot(d);
        T dot_w = -normal.dot(w);

        if (dot_d == 0.0)
            return ray.start;

        return ray.start + d * (dot_w / dot_d);
    }
    
    Vector3<T> normal;
    T distance;
};

typedef Plane<float> Plane3f;
typedef Plane<double> Plane3d;

};//namespace sp

#endif//SP2_MATH_RAY_H
