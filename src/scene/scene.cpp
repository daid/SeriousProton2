#include <sp2/scene/scene.h>
#include <sp2/scene/node.h>

namespace sp {

PVector<Scene> Scene::scenes;

Scene::Scene()
{
    root = new SceneNode(this);
    collision_world2d = nullptr;
    enabled = true;
    
    scenes.add(this);
}

};//!namespace sp
