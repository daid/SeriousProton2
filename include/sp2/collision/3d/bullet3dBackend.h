#ifndef SP2_COLLISION_3D_BULLET_BACKEND_H
#define SP2_COLLISION_3D_BULLET_BACKEND_H

#include <sp2/collision/backend.h>

class b2World;
class b2Body;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btRigidBody;

namespace sp {
namespace collision {

class Shape3D;

class BulletBackend : public Backend
{
public:
    BulletBackend();
    virtual ~BulletBackend();

    virtual void step(float time_delta) override;
    virtual void postUpdate(float delta) override;
    virtual void destroyBody(void* body) override;
    virtual void getDebugRenderMesh(std::shared_ptr<MeshData>& mesh) override;
    
    virtual void updatePosition(void* body, sp::Vector3d position) override;
    virtual void updateRotation(void* body, float angle) override;
    virtual void updateRotation(void* body, Quaterniond rotation) override;
    virtual void setLinearVelocity(void* body, Vector3d velocity) override;
    virtual void setAngularVelocity(void* body, Vector3d velocity) override;
    virtual Vector3d getLinearVelocity(void* body) override;
    virtual Vector3d getAngularVelocity(void* body) override;

    virtual bool testCollision(void* body, Vector3d position) override;
    virtual bool isSolid(void* body) override;

    virtual void query(Vector2d position, std::function<bool(P<Node> object)> callback_function) override;
    virtual void query(Vector2d position, double range, std::function<bool(P<Node> object)> callback_function) override;
    virtual void query(Rect2d area, std::function<bool(P<Node> object)> callback_function) override;
    virtual void queryAny(Ray2d ray, std::function<bool(P<Node> object, Vector2d hit_location, Vector2d hit_normal)> callback_function) override;
    virtual void queryAll(Ray2d ray, std::function<bool(P<Node> object, Vector2d hit_location, Vector2d hit_normal)> callback_function) override;
private:
    btDefaultCollisionConfiguration* configuration = nullptr;
    btCollisionDispatcher* dispatcher = nullptr;
    btBroadphaseInterface* broadphase = nullptr;
    btSequentialImpulseConstraintSolver* solver = nullptr;
    btDiscreteDynamicsWorld* world = nullptr;
    
    friend class Shape3D;
};

}//namespace collision
}//namespace sp

#endif//SP2_COLLISION_SHAPE_H
