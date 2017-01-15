#include <sp2/scene/scene.h>
#include <sp2/scene/node.h>
#include <sp2/scene/cameraNode.h>
#include <sp2/engine.h>
#include <sp2/logging.h>
#include <sp2/assert.h>
#include <box2d/box2d.h>
#include <private/collision/box2dVector.h>

namespace sp {

PList<Scene> Scene::scenes;

Scene::Scene()
{
    root = new SceneNode(this);
    collision_world2d = nullptr;
    enabled = true;
    
    scenes.add(this);
}

Scene::~Scene()
{
    delete *root;
    if (collision_world2d)
        delete collision_world2d;
}

void Scene::setDefaultCamera(P<CameraNode> camera)
{
    sp2assert(camera->getScene() == this, "Trying to set camera from different scene as default for scene.");
    this->camera = camera;
}

void Scene::update(float delta)
{
    if (root)
        updateNode(delta, *root);
    onUpdate(delta);
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
            {
                CollisionInfo info;
                info.other = collision.node_b;
                info.force = collision.force;
                collision.node_a->onCollision(info);
            }
            if (collision.node_a && collision.node_b)
            {
                CollisionInfo info;
                info.other = collision.node_a;
                info.force = collision.force;
                collision.node_b->onCollision(info);
            }
        }
    }
    if (root)
        fixedUpdateNode(*root);
    onFixedUpdate();
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

void Scene::updateNode(float delta, P<SceneNode> node)
{
    node->onUpdate(delta);
    if (node)
    {
        for(SceneNode* child : node->children)
            updateNode(delta, child);
    }
}

void Scene::fixedUpdateNode(P<SceneNode> node)
{
    node->onFixedUpdate();
    if (node)
    {
        for(SceneNode* child : node->children)
            fixedUpdateNode(child);
    }
}

void Scene::destroyCollisionBody2D(b2Body* collision_body2d)
{
    collision_world2d->DestroyBody(collision_body2d);
}

class Box2DQueryCallback : public b2QueryCallback
{
public:
    std::function<bool(SceneNode* node)> callback;
    
	/// Called for each fixture found in the query AABB.
	/// @return false to terminate the query.
	virtual bool ReportFixture(b2Fixture* fixture)
	{
        SceneNode* node = (SceneNode*)fixture->GetUserData();
        return callback(node);
	}
};

void Scene::queryCollision2D(sp::Vector2d position, double range, std::function<bool(P<SceneNode> object)> callback_function)
{
    if (!collision_world2d)
        return;
    Box2DQueryCallback callback;
    callback.callback = [callback_function, position, range](SceneNode* node) {
        if (length(node->getGlobalPosition2D() - position) <= range)
            return callback_function(node);
        return true;
    };
    b2AABB aabb;
    aabb.lowerBound = b2Vec2(position.x - range, position.y - range);
    aabb.upperBound = b2Vec2(position.x + range, position.y + range);
    collision_world2d->QueryAABB(&callback, aabb);
}

};//!namespace sp
