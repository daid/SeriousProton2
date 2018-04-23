#include <sp2/collision/2d/box.h>

#include <private/collision/box2dVector.h>
#include <Box2D/Box2D.h>

namespace sp {
namespace collision {

Box2D::Box2D(double width, double height, double offset_x, double offset_y)
: width(width), height(height), offset_x(offset_x), offset_y(offset_y)
{
}

void Box2D::createFixture(b2Body* body) const
{
    b2PolygonShape shape;
    shape.SetAsBox(width / 2.0, height / 2.0, b2Vec2(offset_x, offset_y), 0);

    createFixtureOnBody(body, &shape);
}

};//namespace collision
};//namespace sp
