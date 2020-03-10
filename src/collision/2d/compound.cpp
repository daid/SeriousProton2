#include <sp2/collision/2d/compound.h>

#include <private/collision/box2dVector.h>
#include <private/collision/box2d.h>


namespace sp {
namespace collision {

Compound2D::Compound2D(std::initializer_list<const Shape2D*> shapes)
: shapes(shapes)
{
}

void Compound2D::createFixture(b2Body* body) const
{
    for(const auto shape : shapes)
        shape->createFixture(body);
}

}//namespace collision
}//namespace sp
