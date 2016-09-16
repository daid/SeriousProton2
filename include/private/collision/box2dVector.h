#ifndef SP2_PRIVATE_COLLISION_BOX2D_VECTOR_H
#define SP2_PRIVATE_COLLISION_BOX2D_VECTOR_H

#include <SFML/System/Vector2.hpp>
#include <Box2D/Common/b2Math.h>

template<typename T> static inline b2Vec2 tob2Vec2(sf::Vector2<T> v)
{
    return b2Vec2(v.x, v.y);
}

#endif//SP2_PRIVATE_COLLISION_BOX2D_VECTOR_H
