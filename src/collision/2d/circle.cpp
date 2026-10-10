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

void Circle2D::createShape(b2BodyId body) const
{
    b2Circle shape;
    shape.radius = radius;
    shape.center = toVector(center);

    auto shapedef = getShapeDef(body);
    b2CreateCircleShape(body, &shapedef, &shape);
}

}//namespace collision
}//namespace sp
