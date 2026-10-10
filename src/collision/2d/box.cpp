#include <sp2/collision/2d/box.h>

#include <private/collision/box2dVector.h>
#include <private/collision/box2d.h>


namespace sp {
namespace collision {

Box2D::Box2D(double width, double height, double offset_x, double offset_y)
: width(width), height(height), offset_x(offset_x), offset_y(offset_y)
{
}

void Box2D::createShape(b2BodyId body) const
{
    b2Polygon shape = b2MakeOffsetBox(width / 2.0, height / 2.0, b2Vec2{float(offset_x), float(offset_y)}, b2Rot_identity);
    
    auto shapedef = getShapeDef(body);
    b2CreatePolygonShape(body, &shapedef, &shape);
}

}//namespace collision
}//namespace sp
