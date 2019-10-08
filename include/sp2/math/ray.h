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
    
    T closestOnRayTo(const T& point, bool clamp=true) const
    {
        T start_to_point = point - start;
        T ray_vector = end - start;
        
        auto distance_normalized = start_to_point.dot(ray_vector) / ray_vector.dot(ray_vector);
        if (clamp)
        {
            if (distance_normalized < 0) return start;
            if (distance_normalized > 1) return end;
        }
        return start + ray_vector * distance_normalized;
    }
    
    Ray<T> shortestDistance(const Ray<T>& other, bool clamp=true) const
    {
        T v13 = start - other.start;
        T v21 = end - start;
        T v43 = other.end - other.start;
        
        auto d1343 = v13.dot(v43);
        auto d4321 = v43.dot(v21);
        auto d1321 = v13.dot(v21);
        auto d4343 = v43.dot(v43);
        auto d2121 = v21.dot(v21);
        
        auto denom = d2121 * d4343 - d4321 * d4321;
        auto numer = d1343 * d4321 - d1321 * d4343;

        //TODO: Check for div/zero
        auto mua = numer / denom;
        //TODO: Check for div/zero
        auto mub = (d1343 + d4321 * mua) / d4343;
        
        if (clamp)
        {
            if (mua < 0) mua = 0;
            if (mua > 1) mua = 1;
            if (mub < 0) mub = 0;
            if (mub > 1) mub = 1;
        }
        
        return Ray<T>(start + (end - start) * mua, other.start + (other.end - other.start) * mub);
    }
    
    T start;
    T end;
};

typedef Ray<Vector2i> Ray2i;
typedef Ray<Vector2f> Ray2f;
typedef Ray<Vector2d> Ray2d;
typedef Ray<Vector3f> Ray3f;
typedef Ray<Vector3d> Ray3d;

template <typename T> static inline std::ostream& operator<<(std::ostream& stream, const Ray<T>& r)
{
    stream << r.start << "->" << r.end;
    return stream;
}

}//namespace sp

#endif//SP2_MATH_RAY_H
