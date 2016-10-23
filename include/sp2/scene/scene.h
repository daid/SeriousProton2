#ifndef SP2_SCENE_SCENE_H
#define SP2_SCENE_SCENE_H

#include <sp2/pointer.h>
#include <sp2/pointerVector.h>

class b2World;

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
    
    friend class collision::Shape2D;
private:
    P<SceneNode> root;
    P<CameraNode> camera;
    b2World* collision_world2d;
    bool enabled;

public:
    static PVector<Scene> scenes;
};

};//!namespace sp

#endif//SP2_SCENE_SCENE_H
