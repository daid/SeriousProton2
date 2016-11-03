#ifndef SP2_SCENE_SCENE_H
#define SP2_SCENE_SCENE_H

#include <sp2/pointer.h>
#include <sp2/pointerVector.h>

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

    P<SceneNode> getRoot() { return root; }
    P<CameraNode> getCamera() { return camera; }
    void setCamera(P<CameraNode> camera) { this->camera = camera; }
    
    bool isEnabled() { return enabled; }
    void fixedUpdate();
    void update(float delta);
    
    void destroyCollisionBody2D(b2Body* collision_body2d);
    
    friend class collision::Shape2D;
private:
    P<SceneNode> root;
    P<CameraNode> camera;
    b2World* collision_world2d;
    bool enabled;

    void updateNode(float delta, SceneNode* node);
    void fixedUpdateNode(SceneNode* node);
public:
    static PVector<Scene> scenes;
};

};//!namespace sp

#endif//SP2_SCENE_SCENE_H
