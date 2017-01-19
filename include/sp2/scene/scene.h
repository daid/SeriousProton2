#ifndef SP2_SCENE_SCENE_H
#define SP2_SCENE_SCENE_H

#include <sp2/pointer.h>
#include <sp2/pointerList.h>
#include <sp2/math/vector.h>

class b2World;
class b2Body;

namespace sp {
namespace collision {
class Shape2D;
}

class SceneNode;
class CameraNode;
class Scene : public AutoPointerObject
{
public:
    Scene();
    virtual ~Scene();

    P<SceneNode> getRoot() { return root; }
    P<CameraNode> getCamera() { return camera; }
    void setDefaultCamera(P<CameraNode> camera);
    
    void enable() { enabled = true; }
    void disable() { enabled = false; }
    void setEnabled(bool enabled) { this->enabled = enabled; }
    bool isEnabled() { return enabled; }
    void fixedUpdate();
    void postFixedUpdate(float delta);
    void update(float delta);
    
    void queryCollision(Vector2d position, double range, std::function<bool(P<SceneNode> object)> callback_function);
    //Gives a callback for any object being hit by the ray from start to end. In any order.
    //Best used to see if start to end is blocked by anything (line of sight)
    //Return false to stop searching for colliding objects.
    void queryCollisionAny(Vector2d start, Vector2d end, std::function<bool(P<SceneNode> object, Vector2d hit_location, Vector2d hit_normal)> callback_function);
    //Gives a callback for any object being hit by the ray from start to end. In the order from start to end.
    //Best used to trace towards the first object that will be hit by something. (hit trace weapons)
    //Return false to stop searching for colliding objects.
    void queryCollisionAll(Vector2d start, Vector2d end, std::function<bool(P<SceneNode> object, Vector2d hit_location, Vector2d hit_normal)> callback_function);
    
    void destroyCollisionBody2D(b2Body* collision_body2d);
    
    virtual void onUpdate(float delta) {}
    virtual void onFixedUpdate() {}
    
    friend class collision::Shape2D;
    friend class CollisionRenderPass;
private:
    P<SceneNode> root;
    P<CameraNode> camera;
    b2World* collision_world2d;
    bool enabled;

    void updateNode(float delta, P<SceneNode> node);
    void fixedUpdateNode(P<SceneNode> node);
public:
    static PList<Scene> scenes;
};

};//!namespace sp

#endif//SP2_SCENE_SCENE_H
