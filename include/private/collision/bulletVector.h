#ifndef SP2_PRIVATE_COLLISION_BULLET_VECTOR_H
#define SP2_PRIVATE_COLLISION_BULLET_VECTOR_H

#include <sp2/math/vector.h>
#include <sp2/math/quaternion.h>
#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>

template<typename T> static inline btVector3 toVector(const sp::Vector3<T>& v)
{
    return btVector3(v.x, v.y, v.z);
}

template<typename T> static inline sp::Vector3<T> toVector(const btVector3& v)
{
    return sp::Vector3<T>(v.getX(), v.getY(), v.getZ());
}

template<typename T> static inline btQuaternion toQuadernion(const sp::Quaternion<T>& q)
{
    return btQuaternion(q.x, q.y, q.z, q.w);
}

template<typename T> static inline sp::Quaternion<T> toQuadernion(const btQuaternion& q)
{
    return sp::Quaternion<T>(q.getX(), q.getY(), q.getZ(), q.getW());
}

#endif//SP2_PRIVATE_COLLISION_BULLET_VECTOR_H

