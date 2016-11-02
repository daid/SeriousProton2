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

void Scene::update(float delta)
{
    if (root)
        updateNode(delta, *root);
}

void Scene::fixedUpdate()
{
    if (root)
        fixedUpdateNode(*root);
}

void Scene::updateNode(float delta, SceneNode* node)
{
    node->onUpdate(delta);
    for(SceneNode* child : node->getChildren())
        updateNode(delta, child);
}

void Scene::fixedUpdateNode(SceneNode* node)
{
    node->onFixedUpdate();
    for(SceneNode* child : node->getChildren())
        fixedUpdateNode(child);
}

};//!namespace sp
