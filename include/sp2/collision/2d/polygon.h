#ifndef SP2_COLLISION_2D_POLYGON_H
#define SP2_COLLISION_2D_POLYGON_H

#include <sp2/collision/2d/shape.h>
#include <sp2/math/vector2.h>
#include <vector>

namespace sp {
namespace collision {

/**
    The Polygon2D creates a 2D convex polygon for collision.
    The polygon MUST be convex, and no more then 8 points.
    
    This class guarantees that there are no more then 8 points, and that the polygon is proper convex.
 */
class Polygon2D : public Shape2D
{
public:
    Polygon2D();
    
    void add(Vector2f point);
private:
    std::vector<Vector2f> points;

    std::vector<Vector2f> buildConvex() const;
    
    virtual void createFixture(b2Body* body) const override;
};

};//namespace collision
};//namespace sp

#endif//SP2_COLLISION_2D_POLYGON_H
