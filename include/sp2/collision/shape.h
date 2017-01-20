#ifndef SP2_COLLISION_SHAPE_H
#define SP2_COLLISION_SHAPE_H

namespace sp {
class SceneNode;
namespace collision {

class Shape
{
public:
    Shape()
    : type(Type::Dynamic), density(1.0), linear_damping(0.0), angular_damping(0.0)
    {
    }

    enum class Type
    {
        Sensor,    //No collision
        Static,    //Collision, no movement
        Kinematic, //Collision, may be moved
        Dynamic    //Full physics simulation
    };
    
    Type type;
    float density;
    float linear_damping;
    float angular_damping;
private:
    virtual void create(SceneNode* node) const = 0;
    
    friend class sp::SceneNode;
};

};//!namespace collision
};//!namespace sp

#endif//SP2_COLLISION_SHAPE_H
