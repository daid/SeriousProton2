#include <sp2/scene/scene.h>
#include <sp2/scene/node.h>

namespace sp {

Scene::Scene()
{
    root = new SceneNode(this);
}

};//!namespace sp
