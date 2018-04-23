#ifndef SP2_COLLISION_2D_BOX_H
#define SP2_COLLISION_2D_BOX_H

#include <sp2/collision/2d/shape.h>

namespace sp {
namespace collision {

class Box2D : public Shape2D
{
public:
    Box2D(double width, double height, double offset_x=0, double offset_y=0);
private:
    double width;
    double height;
    double offset_x;
    double offset_y;

    virtual void createFixture(b2Body* body) const override;
};

};//namespace collision
};//namespace sp

#endif//SP2_COLLISION_2D_CIRCLE_H
