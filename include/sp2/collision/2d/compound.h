#ifndef SP2_COLLISION_2D_COMPOUND_H
#define SP2_COLLISION_2D_COMPOUND_H

#include <sp2/collision/2d/shape.h>
#include <vector>

namespace sp {
namespace collision {

class Compound2D : public Shape2D
{
public:
    Compound2D(const std::vector<const Shape2D*>& shapes);
    Compound2D(std::vector<const Shape2D*>&& shapes);
    Compound2D(std::initializer_list<const Shape2D*> shapes);

    void add(Shape2D* shape);
private:
    virtual void createFixture(b2Body* body) const override;

    std::vector<const Shape2D*> shapes;
};

}//namespace collision
}//namespace sp

#endif//SP2_COLLISION_2D_COMPOUND_H
