#include <sp2/collision/2d/box2dBackend.h>
#include <sp2/collision/2d/joint.h>
#include <sp2/graphics/meshdata.h>

#include <private/collision/box2dVector.h>
#include <private/collision/box2d.h>

namespace sp {
namespace collision {

class Collision2DDebugRender : public b2Draw
{
public:
    Collision2DDebugRender()
    {
        SetFlags(e_shapeBit);
    }

	virtual void DrawPolygon(const b2Vec2* bvertices, int32 vertexCount, const b2Color& color) override
	{
        Vector3f c(color.r, color.g, color.b);
        int idx0 = vertices.size();
        for(int n=0; n<vertexCount; n++)
            vertices.emplace_back(Vector3f(bvertices[n].x, bvertices[n].y, 0.0f), c, Vector2f());
        for(int n=2; n<vertexCount; n++)
        {
            indices.emplace_back(idx0);
            indices.emplace_back(idx0 + n - 1);
            indices.emplace_back(idx0 + n);
        }
	}

	virtual void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override
	{
        DrawPolygon(vertices, vertexCount, color);
	}

	virtual void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) override
	{
        Vector3f c(color.r, color.g, color.b);

        int index = vertices.size();
        vertices.emplace_back(Vector3f(center.x, center.y, 0.0f), c, Vector2f());
        for(int n=0; n<16; n++)
            vertices.emplace_back(Vector3f(center.x + std::sin(float(n) / 8 * pi) * radius, center.y + std::cos(float(n) / 8 * pi) * radius, 0.0f), c, Vector2f());
        for(int n=0; n<16; n++)
        {
            indices.emplace_back(index);
            indices.emplace_back(index + 1 + ((n + 1) % 16));
            indices.emplace_back(index + 1 + n);
        }
	}
	
	virtual void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) override
	{
        DrawCircle(center, radius, color);
	}
	
	virtual void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override
	{
        Vector3f c(color.r, color.g, color.b);

        Vector2f v0 = toVector<float>(p1);
        Vector2f v1 = toVector<float>(p2);
        Vector2f diff = (v1 - v0).normalized() * 0.2f;
        
        int index = vertices.size();
        vertices.emplace_back(Vector3f(v0.x, v0.y, 0.0f), c, Vector2f());
        vertices.emplace_back(Vector3f(v1.x, v1.y, 0.0f), c, Vector2f());
        vertices.emplace_back(Vector3f((v0.x + v1.x) / 2 - diff.y, (v0.y + v1.y) / 2 - diff.x, 0.0f), c, Vector2f());
        vertices.emplace_back(Vector3f((v0.x + v1.x) / 2 + diff.y, (v0.y + v1.y) / 2 + diff.x, 0.0f), c, Vector2f());

        indices.emplace_back(index);
        indices.emplace_back(index + 1);
        indices.emplace_back(index + 2);
        indices.emplace_back(index);
        indices.emplace_back(index + 1);
        indices.emplace_back(index + 3);
	}

	virtual void DrawTransform(const b2Transform& xf) override
	{
        LOG(Debug, "Called unimplemented function", __FUNCTION__);
	}

	virtual void DrawPoint(const b2Vec2& p, float32 size, const b2Color& color) override
	{
        size *= 0.05;
        int index = vertices.size();
        Vector3f c(color.r, color.g, color.b);
        vertices.emplace_back(Vector3f(p.x - size, p.y - size, 0.0f), c, Vector2f());
        vertices.emplace_back(Vector3f(p.x + size, p.y - size, 0.0f), c, Vector2f());
        vertices.emplace_back(Vector3f(p.x - size, p.y + size, 0.0f), c, Vector2f());
        vertices.emplace_back(Vector3f(p.x + size, p.y + size, 0.0f), c, Vector2f());

        indices.emplace_back(index);
        indices.emplace_back(index + 1);
        indices.emplace_back(index + 2);
        indices.emplace_back(index + 2);
        indices.emplace_back(index + 1);
        indices.emplace_back(index + 3);
	}

    sp::MeshData::Vertices vertices;
    sp::MeshData::Indices indices;
};

class ContactListener : public b2ContactListener
{
public:
	virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override
	{
        checkContact(contact, contact->GetFixtureA(), contact->GetChildIndexA(), contact->GetFixtureB(), 1.0);
        checkContact(contact, contact->GetFixtureB(), contact->GetChildIndexB(), contact->GetFixtureA(), -1.0);
	}
private:
    void checkContact(b2Contact* contact, b2Fixture* fixture, int child_index, b2Fixture* other, float direction)
    {
        if (fixture->GetType() == b2Shape::Type::e_chain)
        {
            b2ChainShape* chain = static_cast<b2ChainShape*>(fixture->GetShape());
            b2EdgeShape edge;
            chain->GetChildEdge(&edge, child_index);
            
            b2WorldManifold world_manifold;
            contact->GetWorldManifold(&world_manifold);
            
            Vector2d contact_normal = toVector<double>(world_manifold.normal);
            Vector2d edge_normal = toVector<double>(b2Mul(fixture->GetBody()->GetTransform().q, b2Vec2(edge.m_vertex2.y - edge.m_vertex1.y, edge.m_vertex1.x - edge.m_vertex2.x))).normalized();
            if (edge_normal.dot(contact_normal) * direction > -0.3) //Touching the proper side of the edge.
                contact->SetEnabled(false);

            Vector2d velocity_diff = toVector<double>(fixture->GetBody()->GetLinearVelocity() - other->GetBody()->GetLinearVelocity());
            if (velocity_diff.dot(edge_normal) * direction >= 0.0)   //Moving towards the edge, not away from it.
                contact->SetEnabled(false);
        }
    }
};

class DestructionListener : public b2DestructionListener
{
	virtual void SayGoodbye(b2Joint* joint) override
	{
        Joint2D* my_joint = static_cast<Joint2D*>(joint->GetUserData());
        if (my_joint)
        {
            my_joint->joint = nullptr;
            delete my_joint;
        }
	}
	
	virtual void SayGoodbye(b2Fixture* fixture) override
	{
	}
};

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

Box2DBackend::Box2DBackend()
{
    world = new b2World(b2Vec2_zero);
    world->SetContactListener(new ContactListener());
    world->SetDestructionListener(new DestructionListener());
}

Box2DBackend::~Box2DBackend()
{
    delete world;
}

void Box2DBackend::step(float time_delta)
{
    world->Step(time_delta, 4, 8);
    
    std::vector<Collision> collisions;
    for(b2Contact* contact = world->GetContactList(); contact; contact = contact->GetNext())
    {
        if (contact->IsTouching() && contact->IsEnabled())
        {
            Node* node_a = static_cast<Node*>(contact->GetFixtureA()->GetUserData());
            Node* node_b = static_cast<Node*>(contact->GetFixtureB()->GetUserData());
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

void Box2DBackend::postUpdate(float delta)
{
    for(b2Body* body = world->GetBodyList(); body; body = body->GetNext())
    {
        Node* node = static_cast<Node*>(body->GetUserData());
        modifyPositionByPhysics(node, toVector<double>(body->GetPosition() + delta * body->GetLinearVelocity()), (body->GetAngle() + body->GetAngularVelocity() * delta) / pi * 180.0);
    }
}

void Box2DBackend::destroyBody(void* body)
{
    world->DestroyBody(static_cast<b2Body*>(body));
}

void Box2DBackend::getDebugRenderMesh(std::shared_ptr<MeshData>& mesh)
{
    Collision2DDebugRender debug_renderer;
    
    world->SetDebugDraw(&debug_renderer);
    world->DrawDebugData();
    world->SetDebugDraw(nullptr);
    
    if (!mesh)
        mesh = MeshData::create(std::move(debug_renderer.vertices), std::move(debug_renderer.indices), MeshData::Type::Dynamic);
    else
        mesh->update(std::move(debug_renderer.vertices), std::move(debug_renderer.indices));
}

void Box2DBackend::updatePosition(void* _body, sp::Vector3d position)
{
    b2Body* body = static_cast<b2Body*>(_body);
    body->SetTransform(b2Vec2(position.x, position.y), body->GetAngle());
}

void Box2DBackend::updateRotation(void* _body, float angle)
{
    b2Body* body = static_cast<b2Body*>(_body);
    body->SetTransform(body->GetPosition(), angle / 180.0 * pi);
}

void Box2DBackend::updateRotation(void* _body, Quaterniond rotation)
{
    b2Body* body = static_cast<b2Body*>(_body);
    body->SetTransform(body->GetPosition(), (rotation * Vector2d(1, 0)).angle() / 180.0 * pi);
}

void Box2DBackend::setLinearVelocity(void* _body, Vector3d velocity)
{
    b2Body* body = static_cast<b2Body*>(_body);
    body->SetLinearVelocity(b2Vec2(velocity.x, velocity.y));
}

void Box2DBackend::setAngularVelocity(void* _body, Vector3d velocity)
{
    b2Body* body = static_cast<b2Body*>(_body);
    body->SetAngularVelocity(velocity.z / 180.0 * pi);
}

Vector3d Box2DBackend::getLinearVelocity(void* _body)
{
    b2Body* body = static_cast<b2Body*>(_body);
    b2Vec2 velocity = body->GetLinearVelocity();
    return Vector3d(velocity.x, velocity.y, 0);
}

Vector3d Box2DBackend::getAngularVelocity(void* _body)
{
    b2Body* body = static_cast<b2Body*>(_body);
    return Vector3d(0, 0, body->GetAngularVelocity() / pi * 180.0f);
}

bool Box2DBackend::testCollision(void* _body, Vector3d position)
{
    b2Body* body = static_cast<b2Body*>(_body);
    b2Vec2 pos(position.x, position.y);
    for(const b2Fixture* f = body->GetFixtureList(); f; f = f->GetNext())
    {
        if (f->TestPoint(pos))
            return true;
    }
    return false;
}

bool Box2DBackend::isSolid(void* _body)
{
    b2Body* body = static_cast<b2Body*>(_body);
    for(const b2Fixture* f = body->GetFixtureList(); f; f = f->GetNext())
    {
        if (!f->IsSensor())
            return true;
    }
    return false;
}
 
class Box2DQueryCallback : public b2QueryCallback
{
public:
    std::function<bool(Node* node)> callback;
    
	/// Called for each fixture found in the query AABB.
	/// @return false to terminate the query.
	virtual bool ReportFixture(b2Fixture* fixture) override
	{
        Node* node = static_cast<Node*>(fixture->GetUserData());
        return callback(node);
	}
};

void Box2DBackend::query(sp::Vector2d position, std::function<bool(P<Node> object)> callback_function)
{
    Box2DQueryCallback callback;
    callback.callback = [callback_function, position](Node* node) {
        if (node->testCollision(position))
            return callback_function(node);
        return true;
    };
    b2AABB aabb;
    aabb.lowerBound = b2Vec2(position.x, position.y);
    aabb.upperBound = b2Vec2(position.x, position.y);
    world->QueryAABB(&callback, aabb);
}

void Box2DBackend::query(sp::Vector2d position, double range, std::function<bool(P<Node> object)> callback_function)
{
    Box2DQueryCallback callback;
    callback.callback = [callback_function, position, range](Node* node) {
        if ((node->getGlobalPosition2D() - position).length() <= range)
            return callback_function(node);
        return true;
    };
    b2AABB aabb;
    aabb.lowerBound = b2Vec2(position.x - range, position.y - range);
    aabb.upperBound = b2Vec2(position.x + range, position.y + range);
    world->QueryAABB(&callback, aabb);
}

void Box2DBackend::query(Rect2d area, std::function<bool(P<Node> object)> callback_function)
{
    Box2DQueryCallback callback;
    callback.callback = [callback_function](Node* node) {
        return callback_function(node);
    };
    b2AABB aabb;
    aabb.lowerBound = b2Vec2(std::min(area.position.x, area.position.x + area.size.x), std::min(area.position.y, area.position.y + area.size.y));
    aabb.upperBound = b2Vec2(std::max(area.position.x, area.position.x + area.size.x), std::max(area.position.y, area.position.y + area.size.y));
    world->QueryAABB(&callback, aabb);
}

class Box2DRayCastCallbackAny : public b2RayCastCallback
{
public:
    std::function<bool(Node* node, Vector2d hit_location, Vector2d hit_normal)> callback;
    
	virtual float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction) override
	{
        Node* node = static_cast<Node*>(fixture->GetUserData());
        if (callback(node, toVector<double>(point), toVector<double>(normal)))
            return -1.0;
        return 0.0;
	}
};

void Box2DBackend::queryAny(Ray2d ray, std::function<bool(P<Node> object, Vector2d hit_location, Vector2d hit_normal)> callback_function)
{
    Box2DRayCastCallbackAny callback;
    callback.callback = callback_function;
    world->RayCast(&callback, toVector(ray.start), toVector(ray.end));
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

	virtual float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction) override
	{
        hits.emplace_back(static_cast<Node*>(fixture->GetUserData()), toVector<double>(point), toVector<double>(normal), fraction);
        return -1.0;
	}
};

void Box2DBackend::queryAll(Ray2d ray, std::function<bool(P<Node> object, Vector2d hit_location, Vector2d hit_normal)> callback_function)
{
    Box2DRayCastCallbackAll callback;
    world->RayCast(&callback, toVector(ray.start), toVector(ray.end));
    
    std::sort(callback.hits.begin(), callback.hits.end());
    
    for(Box2DRayCastCallbackAll::Hit& hit : callback.hits)
    {
        if (!callback_function(hit.node, hit.location, hit.normal))
            return;
    }
}

}//namespace collision
}//namespace sp

