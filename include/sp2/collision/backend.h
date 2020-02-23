#ifndef SP2_COLLISION_BACKEND_H
#define SP2_COLLISION_BACKEND_H

#include <memory>
#include <functional>
#include <vector>
#include <sp2/pointer.h>
#include <sp2/math/vector3.h>
#include <sp2/math/quaternion.h>
#include <sp2/math/rect.h>
#include <sp2/math/ray.h>


namespace sp {
class Node;
class MeshData;
namespace collision {

class Backend
{
public:
    Backend() {}
    virtual ~Backend() {}
    
    virtual void step(float time_delta) = 0;
    virtual void postUpdate(float delta) = 0;
    virtual void destroyBody(void* body) = 0;
    virtual void getDebugRenderMesh(std::vector<std::shared_ptr<MeshData>>& meshes) = 0;
    
    virtual void updatePosition(void* body, Vector3d position) = 0;
    virtual void updateRotation(void* body, float angle) = 0;
    virtual void updateRotation(void* body, Quaterniond rotation) = 0;
    virtual void setLinearVelocity(void* body, Vector3d velocity) = 0;
    virtual void setAngularVelocity(void* body, Vector3d velocity) = 0;
    
    virtual Vector3d getLinearVelocity(void* body) = 0;
    virtual Vector3d getAngularVelocity(void* body) = 0;
    
    virtual bool testCollision(void* body, Vector3d position) = 0;
    virtual bool isSolid(void* body) = 0;

    virtual void query(Vector3d position, std::function<bool(P<Node> object)> callback_function) = 0;
    virtual void query(Vector3d position, double range, std::function<bool(P<Node> object)> callback_function) = 0;
    virtual void query(Rect2d area, std::function<bool(P<Node> object)> callback_function) = 0;
    virtual void queryAny(Ray3d ray, std::function<bool(P<Node> object, Vector3d hit_location, Vector3d hit_normal)> callback_function) = 0;
    virtual void queryAll(Ray3d ray, std::function<bool(P<Node> object, Vector3d hit_location, Vector3d hit_normal)> callback_function) = 0;

protected:
    void* getCollisionBody(sp::P<sp::Node>& node);
    void setCollisionBody(sp::P<sp::Node>& node, void* body);

    void modifyPositionByPhysics(Node* node, sp::Vector2d position, double rotation);
    void modifyPositionByPhysics(Node* node, sp::Vector3d position, Quaterniond rotation);
};

}//namespace collision
}//namespace sp

#endif//SP2_COLLISION_SHAPE_H
