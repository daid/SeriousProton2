#ifndef SP2_COLLISION_2D_CHAINS_H
#define SP2_COLLISION_2D_CHAINS_H

#include <sp2/collision/2d/shape.h>
#include <sp2/math/vector.h>
#include <vector>

namespace sp {
namespace collision {

/**
    Specialized chains shape
    This class is designed to be used as static collision in maps.
    It holds one or more chains or loops that can represend a large amount of collision data efficiently, without requiring a lot of Nodes.
    
    Chains do not work for dynamic collisions, so they should always be static objects.
*/
class Chains2D : public Shape2D
{
public:
    Chains2D();

    typedef std::vector<sp::Vector2f> Path;
    
    std::vector<Path> chains;
    std::vector<Path> loops;

private:
    virtual void createFixture(b2Body* body) const override;
};

}//namespace collision
}//namespace sp

#endif//SP2_COLLISION_2D_CIRCLE_H
