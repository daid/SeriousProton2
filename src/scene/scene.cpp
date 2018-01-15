#include <sp2/scene/scene.h>
#include <sp2/scene/node.h>
#include <sp2/scene/camera.h>
#include <sp2/engine.h>
#include <sp2/logging.h>
#include <sp2/assert.h>
#include <Box2D/Box2D.h>
#include <private/collision/box2dVector.h>

namespace sp {

std::unordered_map<string, P<Scene>> Scene::scene_mapping;
PList<Scene> Scene::scenes;

Scene::Scene(string scene_name)
: scene_name(scene_name)
{
    root = new Node(this);
    collision_world2d = nullptr;
    enabled = true;

    sp2assert(scene_mapping.find(scene_name) == scene_mapping.end(), "Cannot create two scenes with the same name.");
    scene_mapping[scene_name] = this;
    
    scenes.add(this);
}

Scene::~Scene()
{
    delete *root;
    if (collision_world2d)
        delete collision_world2d;

    scene_mapping.erase(scene_name);
}

void Scene::setDefaultCamera(P<Camera> camera)
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
    P<Node> node_a;
    P<Node> node_b;
    float force;
    sp::Vector2d position;
    sp::Vector2d normal;

    Collision(P<Node> node_a, P<Node> node_b, float force, sp::Vector2d position, sp::Vector2d normal)
    : node_a(node_a), node_b(node_b), force(force), position(position), normal(normal)
    {}
};

void Scene::fixedUpdate()
{
    if (root)
        fixedUpdateNode(*root);
    onFixedUpdate();

    if (collision_world2d)
    {
        collision_world2d->Step(Engine::fixed_update_delta, 4, 8);
        
        std::vector<Collision> collisions;
        for(b2Contact* contact = collision_world2d->GetContactList(); contact; contact = contact->GetNext())
        {
            if (contact->IsTouching() && contact->IsEnabled())
            {
                Node* node_a = (Node*)contact->GetFixtureA()->GetUserData();
                Node* node_b = (Node*)contact->GetFixtureB()->GetUserData();
                b2WorldManifold world_manifold;
                contact->GetWorldManifold(&world_manifold);

                float collision_force = 0.0f;
                for (int n = 0; n < contact->GetManifold()->pointCount; n++)
                {
                    collision_force += contact->GetManifold()->points[n].normalImpulse;
                }

                if (contact->GetManifold()->pointCount == 0)
                {
                    b2Manifold manifold;
                    const b2Transform& transform_a = contact->GetFixtureA()->GetBody()->GetTransform();
                    const b2Transform& transform_b = contact->GetFixtureB()->GetBody()->GetTransform();
                    contact->Evaluate(&manifold, transform_a, transform_b);
                    
                    //No actual contact? This seems to happen quite often on sensor to sensor contacts...
                    if (manifold.pointCount < 1)
                        continue;
                    
                    world_manifold.Initialize(&manifold, transform_a, contact->GetFixtureA()->GetShape()->m_radius, transform_b, contact->GetFixtureB()->GetShape()->m_radius);
                }

                sp::Vector2d position = toVector<double>(world_manifold.points[0]);
                collisions.emplace_back(node_a, node_b, collision_force, position, toVector<double>(world_manifold.normal));
            }
        }
        for(Collision& collision : collisions)
        {
            if (collision.node_a && collision.node_b)
            {
                CollisionInfo info;
                info.other = collision.node_b;
                info.force = collision.force;
                info.position = collision.position;
                info.normal = collision.normal;
                collision.node_a->onCollision(info);
            }
            if (collision.node_a && collision.node_b)
            {
                CollisionInfo info;
                info.other = collision.node_a;
                info.force = collision.force;
                info.position = collision.position;
                info.normal = -collision.normal;
                collision.node_b->onCollision(info);
            }
        }
    }
}

void Scene::postFixedUpdate(float delta)
{
    if (collision_world2d)
    {
        for(b2Body* body = collision_world2d->GetBodyList(); body; body = body->GetNext())
        {
            Node* node = (Node*)body->GetUserData();
            node->modifyPositionByPhysics(toVector<double>(body->GetPosition() + delta * body->GetLinearVelocity()), (body->GetAngle() + body->GetAngularVelocity() * delta) / pi * 180.0);
        }
    }
}

void Scene::updateNode(float delta, P<Node> node)
{
    node->onUpdate(delta);
    if (node && node->animation)
        node->animation->update(delta, node->render_data);
    if (node)
    {
        for(Node* child : node->children)
            updateNode(delta, child);
    }
}

void Scene::fixedUpdateNode(P<Node> node)
{
    node->onFixedUpdate();
    if (node)
    {
        for(Node* child : node->children)
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
    std::function<bool(Node* node)> callback;
    
	/// Called for each fixture found in the query AABB.
	/// @return false to terminate the query.
	virtual bool ReportFixture(b2Fixture* fixture)
	{
        Node* node = (Node*)fixture->GetUserData();
        return callback(node);
	}
};

void Scene::queryCollision(sp::Vector2d position, double range, std::function<bool(P<Node> object)> callback_function)
{
    if (!collision_world2d)
        return;
    Box2DQueryCallback callback;
    callback.callback = [callback_function, position, range](Node* node) {
        if (length(node->getGlobalPosition2D() - position) <= range)
            return callback_function(node);
        return true;
    };
    b2AABB aabb;
    aabb.lowerBound = b2Vec2(position.x - range, position.y - range);
    aabb.upperBound = b2Vec2(position.x + range, position.y + range);
    collision_world2d->QueryAABB(&callback, aabb);
}

void Scene::queryCollision(sp::Vector2d position, std::function<bool(P<Node> object)> callback_function)
{
    if (!collision_world2d)
        return;
    Box2DQueryCallback callback;
    callback.callback = [callback_function, position](Node* node) {
        if (node->testCollision(position))
            return callback_function(node);
        return true;
    };
    b2AABB aabb;
    aabb.lowerBound = b2Vec2(position.x, position.y);
    aabb.upperBound = b2Vec2(position.x, position.y);
    collision_world2d->QueryAABB(&callback, aabb);
}

void Scene::queryCollision(Vector2d position_a, Vector2d position_b, std::function<bool(P<Node> object)> callback_function)
{
    Box2DQueryCallback callback;
    callback.callback = [callback_function](Node* node) {
        return callback_function(node);
    };
    b2AABB aabb;
    aabb.lowerBound = b2Vec2(std::min(position_a.x, position_b.x), std::min(position_a.y, position_b.y));
    aabb.upperBound = b2Vec2(std::max(position_a.x, position_b.x), std::max(position_a.y, position_b.y));
    collision_world2d->QueryAABB(&callback, aabb);
}

class Box2DRayCastCallbackAny : public b2RayCastCallback
{
public:
    std::function<bool(Node* node, Vector2d hit_location, Vector2d hit_normal)> callback;
    
	virtual float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction)
	{
        Node* node = (Node*)fixture->GetUserData();
        if (callback(node, toVector<double>(point), toVector<double>(normal)))
            return -1.0;
        return 0.0;
	}
};

void Scene::queryCollisionAny(Vector2d start, Vector2d end, std::function<bool(P<Node> object, Vector2d hit_location, Vector2d hit_normal)> callback_function)
{
    if (!collision_world2d)
        return;
    
    Box2DRayCastCallbackAny callback;
    callback.callback = callback_function;
    collision_world2d->RayCast(&callback, toVector(start), toVector(end));
}

class Box2DRayCastCallbackAll : public b2RayCastCallback
{
public:
    class Hit
    {
    public:
        P<Node> node;
        Vector2d location;
        Vector2d normal;
        float fraction;
        
        Hit(Node* node, Vector2d location, Vector2d normal, float fraction)
        : node(node), location(location), normal(normal), fraction(fraction)
        {
        }
        
        bool operator<(const Hit& other) const
        {
            return fraction < other.fraction;
        }
    };
    
    std::vector<Hit> hits;

	virtual float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction)
	{
        hits.emplace_back((Node*)fixture->GetUserData(), toVector<double>(point), toVector<double>(normal), fraction);
        return -1.0;
	}
};

void Scene::queryCollisionAll(Vector2d start, Vector2d end, std::function<bool(P<Node> object, Vector2d hit_location, Vector2d hit_normal)> callback_function)
{
    if (!collision_world2d)
        return;
    
    Box2DRayCastCallbackAll callback;
    collision_world2d->RayCast(&callback, toVector(start), toVector(end));
    
    std::sort(callback.hits.begin(), callback.hits.end());
    
    for(Box2DRayCastCallbackAll::Hit& hit : callback.hits)
    {
        if (!callback_function(hit.node, hit.location, hit.normal))
            return;
    }
}

};//!namespace sp
