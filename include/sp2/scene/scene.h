#ifndef SP2_SCENE_SCENE_H
#define SP2_SCENE_SCENE_H

#include <sp2/pointer.h>
#include <sp2/pointerList.h>
#include <sp2/string.h>
#include <sp2/math/vector.h>
#include <sp2/math/ray.h>
#include <sp2/io/pointer.h>
#include <sp2/script/bindingObject.h>

#include <unordered_map>

class b2World;
class b2Body;

namespace sp {
namespace collision {
class Shape2D;
}

class Node;
class Camera;
class Scene : public ScriptBindingObject
{
public:
    Scene(string scene_name, int priority=0);
    virtual ~Scene();

    P<Node> getRoot() { return root; }
    P<Camera> getCamera() { return camera; }
    void setDefaultCamera(P<Camera> camera);
    
    void enable() { setEnabled(true); }
    void disable() { setEnabled(false); }
    void setEnabled(bool enabled);
    bool isEnabled() { return enabled; }
    void fixedUpdate();
    void postFixedUpdate(float delta);
    void update(float delta);

    virtual bool onPointerDown(io::Pointer::Button button, Ray3d ray, int id);
    virtual void onPointerDrag(Ray3d ray, int id);
    virtual void onPointerUp(Ray3d ray, int id);

    void queryCollision(sp::Vector2d position, std::function<bool(P<Node> object)> callback_function);
    void queryCollision(Vector2d position, double range, std::function<bool(P<Node> object)> callback_function);
    void queryCollision(Vector2d position_a, Vector2d position_b, std::function<bool(P<Node> object)> callback_function);
    //Gives a callback for any object being hit by the ray from start to end. In any order.
    //Best used to see if start to end is blocked by anything (line of sight)
    //Return false to stop searching for colliding objects.
    void queryCollisionAny(Ray2d ray, std::function<bool(P<Node> object, Vector2d hit_location, Vector2d hit_normal)> callback_function);
    //Gives a callback for any object being hit by the ray from start to end. In the order from start to end.
    //Best used to trace towards the first object that will be hit by something. (hit trace weapons)
    //Return false to stop searching for colliding objects.
    void queryCollisionAll(Ray2d ray, std::function<bool(P<Node> object, Vector2d hit_location, Vector2d hit_normal)> callback_function);
    
    void destroyCollisionBody2D(b2Body* collision_body2d);
    
    virtual void onUpdate(float delta) {}
    virtual void onFixedUpdate() {}
    virtual void onEnable() {}
    virtual void onDisable() {}
    
    string getSceneName() const { return scene_name; }
    
    friend class collision::Shape2D;
    friend class CollisionRenderPass;
private:
    string scene_name;
    
    P<Node> root;
    P<Camera> camera;
    b2World* collision_world2d;
    bool enabled;
    int priority;

    void updateNode(float delta, P<Node> node);
    void fixedUpdateNode(P<Node> node);

    static std::unordered_map<string, P<Scene>> scene_mapping;

public:
    static P<Scene> get(string name) { return scene_mapping[name]; }

    static const PList<Scene>& all() { return scenes; }
private:
    static PList<Scene> scenes;
};

};//!namespace sp

#endif//SP2_SCENE_SCENE_H
