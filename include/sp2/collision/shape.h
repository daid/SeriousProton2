#ifndef SP2_COLLISION_SHAPE_H
#define SP2_COLLISION_SHAPE_H

namespace sp {
class Node;
namespace collision {

class Shape
{
public:
    Shape()
    : type(Type::Dynamic), density(1.0), linear_damping(0.0), angular_damping(0.0), restitution(0.0), friction(0.0), fixed_rotation(false), filter_category(0x0001), filter_mask(0xFFFF)
    {
    }

    enum class Type
    {
        Sensor,    //No collision
        Static,    //Collision, no movement
        Kinematic, //Collision, may be moved
        Dynamic    //Full physics simulation
    };
    
    //Set the category for this collision.
    void setFilterCategory(int category)
    {
        filter_category = 1 << category;
    }

    //Mask away this collision category so we do not collide with any objects in this category.
    void setMaskFilterCategory(int category)
    {
        filter_mask &=~(1 << category);
    }
    
    Type type;
    float density;
    float linear_damping;
    float angular_damping;
    float restitution;
    float friction;
    bool fixed_rotation;

    //Collision filtering. Collision is only allowed to happen if (a->filter_category & b->filter_mask) && (b->filter_category & a->filter_mask)
    //Normally you set 1 bit on the filter_category
    int filter_category;
	int filter_mask;
private:
    virtual void create(Node* node) const = 0;
    
    friend class sp::Node;
};

};//namespace collision
};//namespace sp

#endif//SP2_COLLISION_SHAPE_H
