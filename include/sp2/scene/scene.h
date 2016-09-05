#ifndef SP2_SCENE_SCENE_H
#define SP2_SCENE_SCENE_H

#include <sp2/pointer.h>

namespace sp {

class SceneNode;

class Scene : public AutoPointerObject
{
private:
    P<SceneNode> root;

public:
    Scene();

    P<SceneNode> getRoot() { return root; }
};

};//!namespace sp

#endif//SP2_SCENE_SCENE_H
