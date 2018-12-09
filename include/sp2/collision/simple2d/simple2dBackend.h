#ifndef SP2_COLLISION_SIMPLE2D_SIMPLE2D_BACKEND_H
#define SP2_COLLISION_SIMPLE2D_SIMPLE2D_BACKEND_H

#include <sp2/collision/backend.h>
#include <list>



class b2BroadPhase;
namespace sp {
namespace collision {
class Simple2DShape;
class CollisionPair;

class Simple2DBackend : public Backend
{
public:
    Simple2DBackend();
    virtual ~Simple2DBackend();
    
    virtual void step(float time_delta) override;
    virtual void postUpdate(float delta) override;
    virtual void destroyBody(void* body) override;
    virtual void getDebugRenderMesh(std::shared_ptr<MeshData>& mesh) override;
    
    virtual void updatePosition(void* body, Vector3d position) override;
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
    void* createBody(Node* owner, const Simple2DShape& shape);
    void AddPair(void* body_a, void* body_b); //Callback from the broadphase
    
    b2BroadPhase* broadphase;
    std::list<CollisionPair> collision_pairs;
    
    friend class Simple2DShape;
    friend class ::b2BroadPhase;
};

};//namespace collision
};//namespace sp

#endif//SP2_COLLISION_SIMPLE2D_SIMPLE2D_BACKEND_H
