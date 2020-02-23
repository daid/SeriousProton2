#ifndef SP2_COLLISION_2D_BOX2D_BACKEND_H
#define SP2_COLLISION_2D_BOX2D_BACKEND_H

#include <sp2/collision/backend.h>

class b2World;
class b2Body;

namespace sp {
namespace collision {

class Shape2D;
class Joint2D;
class Shape3D;

class Box2DBackend : public Backend
{
public:
    Box2DBackend();
    virtual ~Box2DBackend();
    
    virtual void step(float time_delta) override;
    virtual void postUpdate(float delta) override;
    virtual void destroyBody(void* body) override;
    virtual void getDebugRenderMesh(std::vector<std::shared_ptr<MeshData>>& meshes) override;
    
    virtual void updatePosition(void* body, Vector3d position) override;
    virtual void updateRotation(void* body, float angle) override;
    virtual void updateRotation(void* body, Quaterniond rotation) override;
    virtual void setLinearVelocity(void* body, Vector3d velocity) override;
    virtual void setAngularVelocity(void* body, Vector3d velocity) override;
    virtual Vector3d getLinearVelocity(void* body) override;
    virtual Vector3d getAngularVelocity(void* body) override;
    
    virtual bool testCollision(void* body, Vector3d position) override;
    virtual bool isSolid(void* body) override;

    virtual void query(Vector3d position, std::function<bool(P<Node> object)> callback_function) override;
    virtual void query(Vector3d position, double range, std::function<bool(P<Node> object)> callback_function) override;
    virtual void query(Rect2d area, std::function<bool(P<Node> object)> callback_function) override;
    virtual void queryAny(Ray3d ray, std::function<bool(P<Node> object, Vector3d hit_location, Vector3d hit_normal)> callback_function) override;
    virtual void queryAll(Ray3d ray, std::function<bool(P<Node> object, Vector3d hit_location, Vector3d hit_normal)> callback_function) override;
private:
    b2World* world = nullptr;

    friend class collision::Shape2D;
    friend class collision::Joint2D;
};

}//namespace collision
}//namespace sp

#endif//SP2_COLLISION_SHAPE_H
