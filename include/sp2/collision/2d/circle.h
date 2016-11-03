#ifndef SP2_COLLISION_2D_CIRCLE_H
#define SP2_COLLISION_2D_CIRCLE_H

#include <sp2/collision/2d/shape.h>

namespace sp {
namespace collision {

class Circle2D : public Shape2D
{
public:
    Circle2D(double radius);
private:
    double radius;

    virtual void createFixture(b2Body* body) const override;
};

};//!namespace collision
};//!namespace sp

#endif//SP2_COLLISION_2D_CIRCLE_H
