#ifndef SP2_COLLISION_2D_SHAPE_H
#define SP2_COLLISION_2D_SHAPE_H

#include <sp2/collision/shape.h>

class b2Body;
class b2Shape;

namespace sp {
namespace collision {

class Shape2D : public Shape
{
private:
    virtual void create(Node* node) const override;
    virtual void createFixture(b2Body* body) const = 0;

    friend class Compound2D;
protected:
    void createFixtureOnBody(b2Body* body, b2Shape* shape) const;
};

}//namespace collision
}//namespace sp

#endif//SP2_COLLISION_2D_SHAPE_H
