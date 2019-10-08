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
    Rect(T x, T y, T w, T h) : position(x, y), size(w, h) {}
    
    sp::Vector2<T> center()
    {
        return position + size / T(2);
    }
    
    bool contains(Vector2<T> p) const
    {
        return p.x >= position.x && p.x <= position.x + size.x && p.y >= position.y && p.y <= position.y + size.y;
    }
    
    bool overlaps(const Rect<T>& other) const
    {
        return position.x + size.x >= other.position.x && other.position.x + other.size.x >= position.x &&
            position.y + size.y >= other.position.y && other.position.y + other.size.y >= position.y;
    }
    
    void growToInclude(Vector2<T> p)
    {
        if (p.x < position.x)
        {
            size.x = size.x - p.x + position.x;
            position.x = p.x;
        }
        if (p.y < position.y)
        {
            size.y = size.y - p.y + position.y;
            position.y = p.y;
        }
        if (p.x > position.x + size.x)
            size.x = p.x - position.x;
        if (p.y > position.y + size.y)
            size.y = p.y - position.y;
    }
    
    void shrinkToFitWithin(const Rect<T>& other)
    {
        if (position.x < other.position.x)
        {
            size.x -= other.position.x - position.x;
            position.x = other.position.x;
        }
        if (position.y < other.position.y)
        {
            size.y -= other.position.y - position.y;
            position.y = other.position.y;
        }
        if (position.x + size.x > other.position.x + other.size.x)
            size.x -= (position.x + size.x) - (other.position.x + other.size.x);
        if (position.y + size.y > other.position.y + other.size.y)
            size.y -= (position.y + size.y) - (other.position.y + other.size.y);
        if (size.x < 0)
        {
            position.x += size.x;
            size.x = 0;
        }
        if (size.y < 0)
        {
            position.y += size.y;
            size.y = 0;
        }
    }
    
    Vector2<T> position;
    Vector2<T> size;
};

typedef Rect<int> Rect2i;
typedef Rect<float> Rect2f;
typedef Rect<double> Rect2d;

template <typename T> static inline std::ostream& operator<<(std::ostream& stream, const Rect<T>& r)
{
    stream << r.position << "->" << r.size;
    return stream;
}

}//namespace sp

#endif//SP2_MATH_RECT_H
