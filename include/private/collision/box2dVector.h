#ifndef SP2_PRIVATE_COLLISION_BOX2D_VECTOR_H
#define SP2_PRIVATE_COLLISION_BOX2D_VECTOR_H

#include <sp2/math/vector.h>
#include "box2d.h"

template<typename T> static inline b2Vec2 toVector(const sp::Vector2<T>& v)
{
    return b2Vec2(v.x, v.y);
}

template<typename T> static inline sp::Vector2<T> toVector(const b2Vec2& v)
{
    return sp::Vector2<T>(v.x, v.y);
}

#endif//SP2_PRIVATE_COLLISION_BOX2D_VECTOR_H
