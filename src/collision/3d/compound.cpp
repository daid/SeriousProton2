#include <sp2/collision/3d/compound.h>

#include <private/collision/bulletVector.h>
#include <private/collision/bullet.h>

#include <memory>

namespace sp {
namespace collision {

class BulletCompoundShapeOwner : public btCompoundShape
{
public:
    std::vector<std::unique_ptr<btCollisionShape>> shapes;
};

Compound3D::Compound3D(std::initializer_list<Entry> shapes)
: shapes(shapes)
{
}

btCollisionShape* Compound3D::createShape() const
{
    BulletCompoundShapeOwner* shape = new BulletCompoundShapeOwner();
    for(const auto& e : shapes)
    {
        btTransform t;
        t.setOrigin(toVector(e.position));
        t.setRotation(toQuadernion(e.rotation));
        auto sub_shape = e.shape->createShape();
        shape->shapes.push_back(std::unique_ptr<btCollisionShape>(sub_shape));
        shape->addChildShape(t, sub_shape);
    }
    return shape;
}

}//namespace collision
}//namespace sp
