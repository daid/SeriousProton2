#include <sp2/scene/scene.h>
#include <sp2/scene/node.h>
#include <sp2/engine.h>
#include <sp2/logging.h>
#include <box2d/box2d.h>
#include <private/collision/box2dVector.h>

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
    if (collision_world2d)
    {
        collision_world2d->Step(Engine::fixed_update_delta, 4, 8);
        for(b2Contact* contact = collision_world2d->GetContactList(); contact; contact = contact->GetNext())
        {
            
        }
        for(b2Body* body = collision_world2d->GetBodyList(); body; body = body->GetNext())
        {
            SceneNode* node = (SceneNode*)body->GetUserData();
            node->modifyPositionByPhysics(toVector<double>(body->GetPosition()), body->GetAngle() / pi * 180.0);
        }
    }
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

void Scene::destroyCollisionBody2D(b2Body* collision_body2d)
{
    collision_world2d->DestroyBody(collision_body2d);
}

};//!namespace sp
