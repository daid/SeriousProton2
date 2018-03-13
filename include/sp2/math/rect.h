#ifndef SP2_MATH_RECT_H
#define SP2_MATH_RECT_H

#include <cmath>
#include <sp2/math/vector.h>

namespace sp {

template<typename T> class Rect
{
public:
    Rect() {}
    Rect(Vector2<T> position, Vector2<T> size) : position(position), size(size) {}
    Rect(T x, T y, T w, T h) : position(x, y), size(x, y) {}
    
    Vector2<T> position;
    Vector2<T> size;
};

typedef Rect<float> Rect2f;
typedef Rect<double> Rect2d;

template <typename T> static inline std::ostream& operator<<(std::ostream& stream, const Rect<T>& r)
{
    stream << r.position << "->" << r.size;
    return stream;
}

};//!namespace sp

#endif//SP2_MATH_RECT_H
