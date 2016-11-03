#include <sp2/collision/2d/circle.h>

#include <private/collision/box2dVector.h>
#include <Box2D/Box2D.h>

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

    b2FixtureDef shapeDef;
    shapeDef.shape = &shape;
    shapeDef.density = 1.0;
    shapeDef.friction = 0.0;
    shapeDef.isSensor = (type == Type::Sensor);
    body->CreateFixture(&shapeDef);
}

};//!namespace collision
};//!namespace sp
