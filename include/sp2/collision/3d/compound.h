#ifndef SP2_COLLISION_3D_COMPOUND_H
#define SP2_COLLISION_3D_COMPOUND_H

#include <sp2/collision/3d/shape.h>
#include <sp2/math/vector3.h>
#include <sp2/math/quaternion.h>
#include <vector>

namespace sp {
namespace collision {

class Compound3D : public Shape3D
{
public:
    class Entry
    {
    public:
        const Shape3D* shape;
        sp::Vector3d position;
        sp::Quaterniond rotation;
    };

    Compound3D();
    Compound3D(std::initializer_list<Entry> shapes);

    void add(const Shape3D* shape, sp::Vector3d position, sp::Quaterniond rotation);
private:
    btCollisionShape* createShape() const override;

    std::vector<Entry> shapes;
};

}//namespace collision
}//namespace sp

#endif//SP2_COLLISION_2D_COMPOUND_H
