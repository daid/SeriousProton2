#ifndef SP2_COLLISION_SIMPLE2D_SHAPE_H
#define SP2_COLLISION_SIMPLE2D_SHAPE_H

#include <sp2/collision/shape.h>
#include <sp2/math/rect.h>


namespace sp {
namespace collision {

class Simple2DShape : public Shape
{
public:
    Simple2DShape(sp::Vector2d size)
    : rect(-size / 2.0, size) {}
    Simple2DShape(sp::Rect2d rect)
    : rect(rect.position - rect.size / 2.0, rect.size) {}

    Rect2d rect;

private:
    virtual void create(Node* node) const override;
};

};//namespace collision
};//namespace sp

#endif//SP2_COLLISION_SIMPLE2D_SHAPE_H
