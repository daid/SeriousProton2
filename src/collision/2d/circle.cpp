#include <sp2/collision/2d/circle.h>

#include <private/collision/box2dVector.h>
#include <private/collision/box2d.h>


namespace sp {
namespace collision {

Circle2D::Circle2D(double radius)
: radius(radius)
{
}

void Circle2D::createFixture(b2Body* body) const
{
    b2CircleShape shape;
    shape.m_radius = radius;

    createFixtureOnBody(body, &shape);
}

};//namespace collision
};//namespace sp
