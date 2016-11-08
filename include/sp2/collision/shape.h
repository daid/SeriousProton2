#ifndef SP2_COLLISION_SHAPE_H
#define SP2_COLLISION_SHAPE_H

namespace sp {
class SceneNode;
namespace collision {

class Shape
{
public:
    Shape()
    : type(Type::Dynamic), density(1.0)
    {
    }

    enum class Type
    {
        Sensor,
        Static,
        Dynamic
    };
    
    Type type;
    float density;
private:
    virtual void create(SceneNode* node) const = 0;
    
    friend class sp::SceneNode;
};

};//!namespace collision
};//!namespace sp

#endif//SP2_COLLISION_SHAPE_H
