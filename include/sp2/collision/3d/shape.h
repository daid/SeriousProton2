#ifndef SP2_COLLISION_3D_SHAPE_H
#define SP2_COLLISION_3D_SHAPE_H

#include <sp2/collision/shape.h>

class btCollisionShape;

namespace sp {
namespace collision {

class Shape3D : public Shape
{
private:
    virtual void create(Node* node) const override;
    virtual btCollisionShape* createShape() const = 0;
};

};//namespace collision
};//namespace sp

#endif//SP2_COLLISION_3D_SHAPE_H
