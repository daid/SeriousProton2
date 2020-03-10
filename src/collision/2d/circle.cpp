#include <sp2/collision/2d/circle.h>

#include <private/collision/box2dVector.h>
#include <private/collision/box2d.h>


namespace sp {
namespace collision {

Circle2D::Circle2D(double radius)
: radius(radius)
{
}

Circle2D::Circle2D(Vector2d center, double radius)
: radius(radius), center(center)
{
}

void Circle2D::createFixture(b2Body* body) const
{
    b2CircleShape shape;
    shape.m_radius = radius;
    shape.m_p = toVector(center);

    createFixtureOnBody(body, &shape);
}

}//namespace collision
}//namespace sp
