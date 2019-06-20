#ifndef SP2_MATH_PLANE_H
#define SP2_MATH_PLANE_H

#include <cmath>
#include <sp2/math/vector.h>

namespace sp {

template<typename T> class Plane3
{
public:
    Plane3() {}
    Plane3(Vector3<T> normal, T distance) : normal(normal), distance(distance) {}
    
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
    
    Vector3<T> project(const Vector3<T> p)
    {
        return p - normal * normal.dot(p - normal * distance);
    }
    
    bool isAbove(const Vector3<T> p)
    {
        return normal.dot(p - normal * distance) > 0.0;
    }
    
    Vector3<T> normal;
    T distance;
};

typedef Plane3<float> Plane3f;
typedef Plane3<double> Plane3d;

};//namespace sp

#endif//SP2_MATH_RAY_H
