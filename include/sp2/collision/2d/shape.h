#ifndef SP2_COLLISION_2D_SHAPE_H
#define SP2_COLLISION_2D_SHAPE_H

#include <sp2/collision/shape.h>

class b2Body;

namespace sp {
namespace collision {

class Shape2D : public Shape
{
private:
    virtual void create(SceneNode* node) const override;
    virtual void createFixture(b2Body* body) const = 0;
};

};//!namespace collision
};//!namespace sp

#endif//SP2_COLLISION_2D_SHAPE_H
