#ifndef SP2_COLLISION_2D_SHAPE_H
#define SP2_COLLISION_2D_SHAPE_H

#include <sp2/collision/shape.h>

class b2BodyId;
class b2ShapeId;
class b2ShapeDef;

namespace sp {
namespace collision {

class Shape2D : public Shape
{
private:
    virtual void create(Node* node) const override;
    virtual void createShape(b2BodyId body) const = 0;

    friend class Compound2D;
protected:
    b2ShapeDef getShapeDef(b2BodyId body) const;
};

}//namespace collision
}//namespace sp

#endif//SP2_COLLISION_2D_SHAPE_H
