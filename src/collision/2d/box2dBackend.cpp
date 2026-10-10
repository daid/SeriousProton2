#include <sp2/collision/2d/box2dBackend.h>
#include <sp2/collision/2d/joint.h>
#include <sp2/graphics/meshdata.h>

#include <private/collision/box2dVector.h>
#include <private/collision/box2d.h>

namespace sp {
namespace collision {

static Vector3f b2HexColorToVector3f(b2HexColor color)
{
    return {
        float((color & 0xFF0000) >> 16) / 255.0f,
        float((color & 0x00FF00) >> 8) / 255.0f,
        float((color & 0x0000FF) >> 0) / 255.0f
    };
}


class Collision2DDebugRender
{
public:
	static void DrawPolygon(const b2Vec2* bvertices, int vertexCount, b2HexColor color, void* context)
	{
        Collision2DDebugRender* _this = reinterpret_cast<Collision2DDebugRender*>(context);
        Vector3f c = b2HexColorToVector3f(color);
        int idx0 = _this->vertices.size();
        for(int n=0; n<vertexCount; n++)
            _this->vertices.emplace_back(Vector3f(bvertices[n].x, bvertices[n].y, 0.0f), c, Vector2f());
        for(int n=2; n<vertexCount; n++)
        {
            _this->indices.emplace_back(idx0);
            _this->indices.emplace_back(idx0 + n - 1);
            _this->indices.emplace_back(idx0 + n);
        }
	}
    
    static void DrawSolidPolygon(b2Transform transform, const b2Vec2* bvertices, int vertexCount, float radius, b2HexColor color, void* context)
    {
        Collision2DDebugRender* _this = reinterpret_cast<Collision2DDebugRender*>(context);
        Vector3f c = b2HexColorToVector3f(color);
        int idx0 = _this->vertices.size();
        for(int n=0; n<vertexCount; n++)
        {
            auto p = b2TransformPoint(transform, bvertices[n]);
            _this->vertices.emplace_back(Vector3f(p.x, p.y, 0.0f), c, Vector2f());
        }
        for(int n=2; n<vertexCount; n++)
        {
            _this->indices.emplace_back(idx0);
            _this->indices.emplace_back(idx0 + n - 1);
            _this->indices.emplace_back(idx0 + n);
        }
    }

	static void DrawCircle(b2Vec2 center, float radius, b2HexColor color, void* context)
	{
        Collision2DDebugRender* _this = reinterpret_cast<Collision2DDebugRender*>(context);
        Vector3f c = b2HexColorToVector3f(color);

        int index = _this->vertices.size();
        _this->vertices.emplace_back(Vector3f(center.x, center.y, 0.0f), c, Vector2f());
        for(int n=0; n<16; n++)
            _this->vertices.emplace_back(Vector3f(center.x + std::sin(float(n) / 8 * pi) * radius, center.y + std::cos(float(n) / 8 * pi) * radius, 0.0f), c, Vector2f());
        for(int n=0; n<16; n++)
        {
            _this->indices.emplace_back(index);
            _this->indices.emplace_back(index + 1 + ((n + 1) % 16));
            _this->indices.emplace_back(index + 1 + n);
        }
	}
	
	static void DrawSegment(b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* context)
	{
        Collision2DDebugRender* _this = reinterpret_cast<Collision2DDebugRender*>(context);
        Vector3f c = b2HexColorToVector3f(color);

        Vector2f v0 = toVector<float>(p1);
        Vector2f v1 = toVector<float>(p2);
        Vector2f diff = (v1 - v0).normalized() * 0.2f;
        
        int index = _this->vertices.size();
        _this->vertices.emplace_back(Vector3f(v0.x, v0.y, 0.0f), c, Vector2f());
        _this->vertices.emplace_back(Vector3f(v1.x, v1.y, 0.0f), c, Vector2f());
        _this->vertices.emplace_back(Vector3f((v0.x + v1.x) / 2 - diff.y, (v0.y + v1.y) / 2 - diff.x, 0.0f), c, Vector2f());
        _this->vertices.emplace_back(Vector3f((v0.x + v1.x) / 2 + diff.y, (v0.y + v1.y) / 2 + diff.x, 0.0f), c, Vector2f());

        _this->indices.emplace_back(index);
        _this->indices.emplace_back(index + 1);
        _this->indices.emplace_back(index + 2);
        _this->indices.emplace_back(index);
        _this->indices.emplace_back(index + 1);
        _this->indices.emplace_back(index + 3);
	}

	static void DrawPoint(b2Vec2 p, float size, b2HexColor color, void* context)
	{
        Collision2DDebugRender* _this = reinterpret_cast<Collision2DDebugRender*>(context);
        size *= 0.05;
        int index = _this->vertices.size();
        Vector3f c = b2HexColorToVector3f(color);
        _this->vertices.emplace_back(Vector3f(p.x - size, p.y - size, 0.0f), c, Vector2f());
        _this->vertices.emplace_back(Vector3f(p.x + size, p.y - size, 0.0f), c, Vector2f());
        _this->vertices.emplace_back(Vector3f(p.x - size, p.y + size, 0.0f), c, Vector2f());
        _this->vertices.emplace_back(Vector3f(p.x + size, p.y + size, 0.0f), c, Vector2f());

        _this->indices.emplace_back(index);
        _this->indices.emplace_back(index + 1);
        _this->indices.emplace_back(index + 2);
        _this->indices.emplace_back(index + 2);
        _this->indices.emplace_back(index + 1);
        _this->indices.emplace_back(index + 3);
	}

    sp::MeshData::Vertices vertices;
    sp::MeshData::Indices indices;
};

// class DestructionListener : public b2DestructionListener
// {
// 	virtual void SayGoodbye(b2Joint* joint) override
// 	{
//         Joint2D* my_joint = static_cast<Joint2D*>(joint->GetUserData());
//         if (my_joint)
//         {
//             my_joint->joint = nullptr;
//             delete my_joint;
//         }
// 	}
	
// 	virtual void SayGoodbye(b2Fixture* fixture) override
// 	{
// 	}
// };

namespace {
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
}

Box2DBackend::Box2DBackend()
{
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {0.0f, 0.0f};
    world = b2CreateWorld(&worldDef);
    //TODO: Handle joint destruction world->SetDestructionListener(new DestructionListener());
}

Box2DBackend::~Box2DBackend()
{
    b2DestroyWorld(world);
}

void Box2DBackend::step(float time_delta)
{
    b2World_Step(world, time_delta, 4);
    
    std::vector<Collision> collisions;
    std::vector<b2ContactData> contacts;
    for(auto body : bodies) {
        auto cnt = b2Body_GetContactCapacity(body);
        if (contacts.size() < size_t(cnt))
            contacts.resize(cnt);
        cnt = b2Body_GetContactData(body, contacts.data(), contacts.size());
        for(int n=0; n<cnt; n++) {
            if (contacts[n].manifold.pointCount > 0) {
                auto node_a = static_cast<Node*>(b2Shape_GetUserData(contacts[n].shapeIdA));
                auto node_b = static_cast<Node*>(b2Shape_GetUserData(contacts[n].shapeIdB));
                float collision_force = contacts[n].manifold.points[0].totalNormalImpulse;
                auto position = toVector<double>(contacts[n].manifold.points[0].point);
                collisions.emplace_back(node_a, node_b, collision_force, position, toVector<double>(contacts[n].manifold.normal));
            }
        }
    }
    // for(b2Contact* contact = world->GetContactList(); contact; contact = contact->GetNext())
    // {
    //     if (contact->IsTouching() && contact->IsEnabled())
    //     {
    //         Node* node_a = static_cast<Node*>(contact->GetFixtureA()->GetUserData());
    //         Node* node_b = static_cast<Node*>(contact->GetFixtureB()->GetUserData());
    //         b2WorldManifold world_manifold;
    //         contact->GetWorldManifold(&world_manifold);

    //         float collision_force = 0.0f;
    //         for (int n = 0; n < contact->GetManifold()->pointCount; n++)
    //         {
    //             collision_force += contact->GetManifold()->points[n].normalImpulse;
    //         }

    //         if (contact->GetManifold()->pointCount == 0)
    //         {
    //             b2Manifold manifold;
    //             const b2Transform& transform_a = contact->GetFixtureA()->GetBody()->GetTransform();
    //             const b2Transform& transform_b = contact->GetFixtureB()->GetBody()->GetTransform();
    //             contact->Evaluate(&manifold, transform_a, transform_b);
                
    //             //No actual contact? This seems to happen quite often on sensor to sensor contacts...
    //             if (manifold.pointCount < 1)
    //                 continue;
                
    //             world_manifold.Initialize(&manifold, transform_a, contact->GetFixtureA()->GetShape()->m_radius, transform_b, contact->GetFixtureB()->GetShape()->m_radius);
    //         }

    //         sp::Vector2d position = toVector<double>(world_manifold.points[0]);
    //         collisions.emplace_back(node_a, node_b, collision_force, position, toVector<double>(world_manifold.normal));
    //     }
    // }
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
    for(auto body : bodies)
    {
        if (b2Body_IsValid(body)) {
            Node* node = static_cast<Node*>(b2Body_GetUserData(body));
            modifyPositionByPhysics(node, toVector<double>(b2Body_GetPosition(body)) + toVector<double>(b2Body_GetLinearVelocity(body)) * double(delta), b2Rot_GetAngle(b2Body_GetRotation(body)) + (b2Body_GetAngularVelocity(body) * delta) / pi * 180.0);
        }
    }
}

void Box2DBackend::getDebugRenderMesh(std::vector<std::shared_ptr<MeshData>>& meshes)
{
    Collision2DDebugRender debug_renderer;
    b2DebugDraw debug_draw = {
        .DrawPolygonFcn = &Collision2DDebugRender::DrawPolygon,
        .DrawSolidPolygonFcn = &Collision2DDebugRender::DrawSolidPolygon,
        .DrawCircleFcn = &Collision2DDebugRender::DrawCircle,
        .DrawSegmentFcn = &Collision2DDebugRender::DrawSegment,
        .DrawPointFcn = &Collision2DDebugRender::DrawPoint,
        .drawShapes = true,
        .context = &debug_renderer,
    };
    b2World_Draw(world, &debug_draw);
    
    if (meshes.size() < 1)
        meshes.push_back(MeshData::create(std::move(debug_renderer.vertices), std::move(debug_renderer.indices), MeshData::Type::Dynamic));
    else
        meshes[0]->update(std::move(debug_renderer.vertices), std::move(debug_renderer.indices));
}

void Box2DBackend::updatePosition(void* _body, sp::Vector3d position)
{
    auto body = getBodyId(_body);
    b2Body_SetTransform(body, toVector(position), b2Body_GetRotation(body));
}

void Box2DBackend::updateRotation(void* _body, float angle)
{
    auto body = getBodyId(_body);
    b2Body_SetTransform(body, b2Body_GetPosition(body), b2MakeRot(angle / 180.0 * pi));
}

void Box2DBackend::updateRotation(void* _body, Quaterniond rotation)
{
    auto body = getBodyId(_body);
    float angle = (rotation * Vector2d(1, 0)).angle();
    b2Body_SetTransform(body, b2Body_GetPosition(body), b2MakeRot(angle / 180.0 * pi));
}

void Box2DBackend::setLinearVelocity(void* _body, Vector3d velocity)
{
    auto body = getBodyId(_body);
    b2Body_SetLinearVelocity(body, toVector(velocity));
}

void Box2DBackend::setAngularVelocity(void* _body, Vector3d velocity)
{
    auto body = getBodyId(_body);
    b2Body_SetAngularVelocity(body, velocity.z / 180.0 * pi);
}

Vector3d Box2DBackend::getLinearVelocity(void* _body)
{
    auto body = getBodyId(_body);
    b2Vec2 velocity = b2Body_GetLinearVelocity(body);
    return Vector3d(velocity.x, velocity.y, 0);
}

Vector3d Box2DBackend::getAngularVelocity(void* _body)
{
    auto body = getBodyId(_body);
    return Vector3d(0, 0, b2Body_GetAngularVelocity(body) / pi * 180.0f);
}

bool Box2DBackend::testCollision(void* _body, Vector3d position)
{
    auto body = getBodyId(_body);
    b2Vec2 pos{float(position.x), float(position.y)};
    auto shapeCount = b2Body_GetShapeCount(body);
    b2ShapeId shapes[shapeCount];
    b2Body_GetShapes(body, shapes, shapeCount);
    for(int idx=0; idx<shapeCount; idx++)
    {
        if (b2Shape_TestPoint(shapes[idx], pos))
            return true;
    }
    return false;
}

bool Box2DBackend::isSolid(void* _body)
{
    auto body = getBodyId(_body);
    auto shapeCount = b2Body_GetShapeCount(body);
    b2ShapeId shapes[shapeCount];
    b2Body_GetShapes(body, shapes, shapeCount);
    for(int idx=0; idx<shapeCount; idx++)
    {
        if (!b2Shape_IsSensor(shapes[idx]))
            return true;
    }
    return false;
}

void Box2DBackend::query(sp::Vector3d position, std::function<bool(P<Node> object)> callback_function)
{
    std::function<bool(Node*)> callback = [&callback_function, position](Node* node) -> bool {
        if (node->testCollision(position))
            return callback_function(node);
        return true;
    };
    b2AABB aabb;
    aabb.lowerBound = toVector(position);
    aabb.upperBound = toVector(position);
    b2QueryFilter filter = b2DefaultQueryFilter();
    b2World_OverlapAABB(world, aabb, filter, [](b2ShapeId shape, void* context) -> bool {
        auto callback_ptr = static_cast<std::function<bool(Node*)>*>(context);
        return (*callback_ptr)(static_cast<Node*>(b2Shape_GetUserData(shape)));
    }, &callback);
}

void Box2DBackend::query(sp::Vector3d position, double range, std::function<bool(P<Node> object)> callback_function)
{
    sp::Vector2d p(position.x, position.y);
    std::function<bool(Node*)> callback = [&callback_function, p, range](Node* node) {
        if ((node->getGlobalPosition2D() - p).length() <= range)
            return callback_function(node);
        return true;
    };
    b2AABB aabb;
    aabb.lowerBound = b2Vec2{float(position.x - range), float(position.y - range)};
    aabb.upperBound = b2Vec2{float(position.x + range), float(position.y + range)};
    b2QueryFilter filter = b2DefaultQueryFilter();
    b2World_OverlapAABB(world, aabb, filter, [](b2ShapeId shape, void* context) -> bool {
        auto callback_ptr = static_cast<std::function<bool(Node*)>*>(context);
        return (*callback_ptr)(static_cast<Node*>(b2Shape_GetUserData(shape)));
    }, &callback);
}

void Box2DBackend::query(Rect2d area, std::function<bool(P<Node> object)> callback_function)
{
    std::function<bool(Node*)> callback = [&callback_function](Node* node) {
        return callback_function(node);
    };
    b2AABB aabb;
    aabb.lowerBound = b2Vec2{float(std::min(area.position.x, area.position.x + area.size.x)), float(std::min(area.position.y, area.position.y + area.size.y))};
    aabb.upperBound = b2Vec2{float(std::max(area.position.x, area.position.x + area.size.x)), float(std::max(area.position.y, area.position.y + area.size.y))};
    b2QueryFilter filter = b2DefaultQueryFilter();
    b2World_OverlapAABB(world, aabb, filter, [](b2ShapeId shape, void* context) -> bool {
        auto callback_ptr = static_cast<std::function<bool(Node*)>*>(context);
        return (*callback_ptr)(static_cast<Node*>(b2Shape_GetUserData(shape)));
    }, &callback);
}

void Box2DBackend::queryAny(Ray3d ray, std::function<bool(P<Node> object, Vector3d hit_location, Vector3d hit_normal)> callback_function)
{
    std::function<float(Node*, Vector3d, Vector3d)> callback = [&callback_function](Node* node, Vector3d hit_location, Vector3d hit_normal) {
        return callback_function(node, hit_location, hit_normal) ? -1.0f : 0.0f;
    };

    b2QueryFilter filter = b2DefaultQueryFilter();
    b2World_CastRay(world, toVector(ray.start), toVector(ray.end - ray.start), filter, [](b2ShapeId shape, b2Vec2 point, b2Vec2 normal, float fraction, void* context) -> float {
        auto callback_ptr = static_cast<std::function<float(Node*, Vector3d, Vector3d)>*>(context);
        return (*callback_ptr)(static_cast<Node*>(b2Shape_GetUserData(shape)), toVector3<double>(point), toVector3<double>(normal));
    }, &callback);
}

void Box2DBackend::queryAll(Ray3d ray, std::function<bool(P<Node> object, Vector3d hit_location, Vector3d hit_normal)> callback_function)
{
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

    std::function<float(Node*, Vector2d, Vector2d, float)> callback = [&hits](Node* node, Vector2d hit_location, Vector2d hit_normal, float fraction) {
        hits.emplace_back(node, hit_location, hit_normal, fraction);
        return -1.0;
    };

    b2QueryFilter filter = b2DefaultQueryFilter();
    b2World_CastRay(world, toVector(ray.start), toVector(ray.end - ray.start), filter, [](b2ShapeId shape, b2Vec2 point, b2Vec2 normal, float fraction, void* context) -> float {
        auto callback_ptr = static_cast<std::function<float(Node*, Vector2d, Vector2d, float)>*>(context);
        return (*callback_ptr)(static_cast<Node*>(b2Shape_GetUserData(shape)), toVector<double>(point), toVector<double>(normal), fraction);
    }, &callback);

    std::sort(hits.begin(), hits.end());
    
    for(auto& hit : hits)
    {
        if (!callback_function(hit.node, sp::Vector3d(hit.location.x, hit.location.y, 0), sp::Vector3d(hit.normal.x, hit.normal.y, 0)))
            return;
    }
}

void* Box2DBackend::addBodyId(b2BodyId body)
{
    if (free_body_start != std::numeric_limits<uint32_t>::max())
    {
        auto idx = free_body_start;
        free_body_start = bodies[idx].index1;
        bodies[idx] = body;
        return reinterpret_cast<void*>(idx + 1);
    }
    bodies.push_back(body);
    return reinterpret_cast<void*>(bodies.size());
}

void Box2DBackend::destroyBody(void* _body)
{
    auto idx = reinterpret_cast<intptr_t>(_body);
    if (idx < 1 || size_t(idx) > bodies.size()) return;

    b2DestroyBody(bodies[idx-1]);
    bodies[idx-1].index1 = free_body_start;
    bodies[idx-1].world0 = std::numeric_limits<uint16_t>::max();
    free_body_start = idx-1;
}

b2BodyId Box2DBackend::getBodyId(void* body)
{
    auto idx = reinterpret_cast<intptr_t>(body);
    if (idx < 1 || size_t(idx) > bodies.size()) return b2_nullBodyId;
    return bodies[idx-1];
}

}//namespace collision
}//namespace sp

