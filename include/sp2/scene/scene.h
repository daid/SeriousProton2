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
class Scene : public AutoPointerObject
{
public:
    Scene();

    P<SceneNode> getRoot() { return root; }
    
    friend class collision::Shape2D;
private:
    P<SceneNode> root;
    b2World* collision_world2d;
    bool enabled;

    static PVector<Scene> scenes;
};

};//!namespace sp

#endif//SP2_SCENE_SCENE_H
