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
    : rect(Vector2d(0, 0), size) {}
    Simple2DShape(sp::Rect2d rect)
    : rect(rect) {}

    Rect2d rect;

private:
    virtual void create(Node* node) const override;
};

};//namespace collision
};//namespace sp

#endif//SP2_COLLISION_SIMPLE2D_SHAPE_H
