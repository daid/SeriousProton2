#ifndef SP2_COLLISION_2D_CIRCLE_H
#define SP2_COLLISION_2D_CIRCLE_H

#include <sp2/collision/2d/shape.h>
#include <sp2/math/vector2.h>

namespace sp {
namespace collision {

class Circle2D : public Shape2D
{
public:
    Circle2D(double radius);
    Circle2D(Vector2d center, double radius);
private:
    double radius;
    Vector2d center;

    virtual void createFixture(b2Body* body) const override;
};

}//namespace collision
}//namespace sp

#endif//SP2_COLLISION_2D_CIRCLE_H
