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

Scene::~Scene()
{
    if (collision_world2d)
        delete collision_world2d;
}

void Scene::update(float delta)
{
    if (root)
        updateNode(delta, *root);
}

class Collision
{
public:
    P<SceneNode> node_a;
    P<SceneNode> node_b;
    float force;

    Collision(P<SceneNode> node_a, P<SceneNode> node_b, float force)
    : node_a(node_a), node_b(node_b), force(force)
    {}
};

void Scene::fixedUpdate()
{
    if (collision_world2d)
    {
        collision_world2d->Step(Engine::fixed_update_delta, 4, 8);
        std::vector<Collision> collisions;
        for(b2Contact* contact = collision_world2d->GetContactList(); contact; contact = contact->GetNext())
        {
            if (contact->IsTouching() && contact->IsEnabled())
            {
                SceneNode* node_a = (SceneNode*)contact->GetFixtureA()->GetUserData();
                SceneNode* node_b = (SceneNode*)contact->GetFixtureB()->GetUserData();

                float collision_force = 0.0f;
                for (int n = 0; n < contact->GetManifold()->pointCount; n++)
                {
                    collision_force += contact->GetManifold()->points[n].normalImpulse;
                }
                collisions.push_back(Collision(node_a, node_b, collision_force));
            }
        }
        for(Collision& collision : collisions)
        {
            if (collision.node_a && collision.node_b)
                collision.node_a->onCollision(collision.node_b, collision.force);
            if (collision.node_a && collision.node_b)
                collision.node_b->onCollision(collision.node_a, collision.force);
        }
    }
    if (root)
        fixedUpdateNode(*root);
}

void Scene::postFixedUpdate(float delta)
{
    if (collision_world2d)
    {
        for(b2Body* body = collision_world2d->GetBodyList(); body; body = body->GetNext())
        {
            SceneNode* node = (SceneNode*)body->GetUserData();
            node->modifyPositionByPhysics(toVector<double>(body->GetPosition() + delta * body->GetLinearVelocity()), (body->GetAngle() + body->GetAngularVelocity() * delta) / pi * 180.0);
        }
    }
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
