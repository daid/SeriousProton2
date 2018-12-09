#include <sp2/collision/simple2d/simple2dBackend.h>
#include <sp2/collision/simple2d/shape.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/scene/node.h>

#include <Box2D/Collision/b2BroadPhase.h>
#include <private/collision/box2dVector.h>


namespace sp {
namespace collision {

class Simple2DBody;
class CollisionPair
{
public:
    Simple2DBody* body_a;
    Simple2DBody* body_b;
};

class Simple2DBody
{
public:
    Shape::Type type;
    Rect2d shape;
    Node* owner;
    int broadphase_proxy;
    int filter_category;
    int filter_mask;
    
    b2AABB getAABB()
    {
        b2AABB aabb;
        sp::Vector2d position = owner->getPosition2D();
        aabb.lowerBound = toVector<double>(position + shape.position);
        aabb.upperBound = toVector<double>(position + shape.position + shape.size);
        return aabb;
    }
};

Simple2DBackend::Simple2DBackend()
{
    broadphase = new b2BroadPhase();
}

Simple2DBackend::~Simple2DBackend()
{
    delete broadphase;
}

void Simple2DBackend::step(float time_delta)
{
    broadphase->UpdatePairs(this);

    collision_pairs.remove_if([this](CollisionPair& pair)
    {
        return !broadphase->TestOverlap(pair.body_a->broadphase_proxy, pair.body_b->broadphase_proxy);
    });
    
    //We make a copy of the collision list, as onCollision could delete an object, which can erase items from the collision pairs list.
    std::list<CollisionPair> pairs = collision_pairs;
    for(auto& pair : pairs)
    {
        Rect2d rect_a = pair.body_a->shape;
        Rect2d rect_b = pair.body_b->shape;
        rect_a.position += pair.body_a->owner->getPosition2D();
        rect_b.position += pair.body_b->owner->getPosition2D();
        if (rect_a.overlaps(rect_b))
        {
            CollisionInfo info;
            //TODO: Handle static to dynamic object collision, push the dynamic out of the static object.
            double overlap_x = std::min(rect_a.position.x + rect_a.size.x, rect_b.position.x + rect_b.size.x) - std::max(rect_a.position.x, rect_b.position.x);
            double overlap_y = std::min(rect_a.position.y + rect_a.size.y, rect_b.position.y + rect_b.size.y) - std::max(rect_a.position.y, rect_b.position.y);
            if (overlap_x > overlap_y)
            {
                info.force = overlap_y;
                if (rect_a.position.y + rect_a.size.y / 2.0 < rect_b.position.y + rect_b.size.y / 2.0)
                    info.normal = sp::Vector2d(0, -1);
                else
                    info.normal = sp::Vector2d(0, 1);
            }
            else
            {
                info.force = overlap_x;
                if (rect_a.position.x + rect_a.size.x / 2.0 < rect_b.position.x + rect_b.size.x / 2.0)
                    info.normal = sp::Vector2d(-1, 0);
                else
                    info.normal = sp::Vector2d(1, 0);
            }
            
            if (pair.body_a->type == Shape::Type::Dynamic && isSolid(pair.body_b))
                modifyPositionByPhysics(pair.body_a->owner, pair.body_a->owner->getPosition2D() + info.normal * double(info.force), 0);
            if (pair.body_b->type == Shape::Type::Dynamic && isSolid(pair.body_a))
                modifyPositionByPhysics(pair.body_a->owner, pair.body_a->owner->getPosition2D() - info.normal * double(info.force), 0);
            
            //Move the points to P<> pointers, as the onCollision could delete one of the objects.
            P<Node> node_a = pair.body_a->owner;
            P<Node> node_b = pair.body_a->owner;

            info.other = node_b;
            node_a->onCollision(info);
            if (node_b)
            {
                info.other = node_a;
                info.normal = -info.normal;
                node_b->onCollision(info);
            }
        }
    }
}

void Simple2DBackend::postUpdate(float delta)
{
}

void Simple2DBackend::destroyBody(void* _body)
{
    Simple2DBody* body = static_cast<Simple2DBody*>(_body);
    broadphase->DestroyProxy(body->broadphase_proxy);
    collision_pairs.remove_if([body](CollisionPair& pair)
    {
        return pair.body_a == body || pair.body_b == body;
    });
    delete body;
}

void Simple2DBackend::getDebugRenderMesh(std::shared_ptr<MeshData>& mesh)
{
    b2AABB bounds;
    bounds.lowerBound.x = bounds.lowerBound.y = -std::numeric_limits<float>::infinity();
    bounds.upperBound.x = bounds.upperBound.y = std::numeric_limits<float>::infinity();
    MeshData::Vertices vertices;
    MeshData::Indices indices;
    query_callback = [&vertices, &indices](void* _body)
    {
        Simple2DBody* body = static_cast<Simple2DBody*>(_body);

        Vector2d p0 = body->owner->getPosition2D() + body->shape.position;
        Vector2d p1 = p0 + body->shape.size;
        Vector3f c(1, 1, 1);
        int index = vertices.size();
        vertices.emplace_back(Vector3f(p0.x, p0.y, 0.0f), c, Vector2f());
        vertices.emplace_back(Vector3f(p1.x, p0.y, 0.0f), c, Vector2f());
        vertices.emplace_back(Vector3f(p0.x, p1.y, 0.0f), c, Vector2f());
        vertices.emplace_back(Vector3f(p1.x, p1.y, 0.0f), c, Vector2f());

        indices.emplace_back(index);
        indices.emplace_back(index + 1);
        indices.emplace_back(index + 2);
        indices.emplace_back(index + 2);
        indices.emplace_back(index + 1);
        indices.emplace_back(index + 3);
        return true;
    };
    broadphase->Query(this, bounds);
    query_callback = nullptr;

    if (!mesh)
        mesh = MeshData::create(std::move(vertices), std::move(indices), MeshData::Type::Dynamic);
    else
        mesh->update(std::move(vertices), std::move(indices));
}

void Simple2DBackend::updatePosition(void* _body, Vector3d position)
{
    Simple2DBody* body = static_cast<Simple2DBody*>(_body);
    broadphase->MoveProxy(body->broadphase_proxy, body->getAABB(), b2Vec2(0, 0));
}

void Simple2DBackend::updateRotation(void* _body, float angle)
{
}

void Simple2DBackend::updateRotation(void* _body, Quaterniond rotation)
{
}

void Simple2DBackend::setLinearVelocity(void* _body, Vector3d velocity)
{
}

void Simple2DBackend::setAngularVelocity(void* _body, Vector3d velocity)
{
}

Vector3d Simple2DBackend::getLinearVelocity(void* _body)
{
    return Vector3d(0, 0, 0);
}

Vector3d Simple2DBackend::getAngularVelocity(void* body)
{
    return Vector3d(0, 0, 0);
}

bool Simple2DBackend::testCollision(void* _body, Vector3d position)
{
    Simple2DBody* body = static_cast<Simple2DBody*>(_body);
    return body->shape.contains(Vector2d(position.x, position.y) - body->owner->getPosition2D());
}

bool Simple2DBackend::isSolid(void* _body)
{
    Simple2DBody* body = static_cast<Simple2DBody*>(_body);
    return body->type == Shape::Type::Static || body->type == Shape::Type::Kinematic;
}

void Simple2DBackend::query(Vector2d position, std::function<bool(P<Node> object)> callback_function)
{
    b2AABB bounds;
    bounds.lowerBound.x = bounds.upperBound.x = position.x;
    bounds.lowerBound.y = bounds.upperBound.y = position.y;
    query_callback = [&callback_function](void* _body)
    {
        return callback_function(static_cast<Simple2DBody*>(_body)->owner);
    };
    broadphase->Query(this, bounds);
    query_callback = nullptr;
}

void Simple2DBackend::query(Vector2d position, double range, std::function<bool(P<Node> object)> callback_function)
{
    b2AABB bounds;
    bounds.lowerBound.x = position.x - range;
    bounds.lowerBound.y = position.y - range;
    bounds.upperBound.x = position.x + range;
    bounds.upperBound.y = position.y + range;
    query_callback = [&callback_function, position, range](void* _body)
    {
        Node* owner = static_cast<Simple2DBody*>(_body)->owner;
        if ((owner->getPosition2D() - position).length() <= range)
            return callback_function(owner);
        return true;
    };
    broadphase->Query(this, bounds);
    query_callback = nullptr;
}

void Simple2DBackend::query(Rect2d area, std::function<bool(P<Node> object)> callback_function)
{
    b2AABB bounds;
    bounds.lowerBound.x = area.position.x;
    bounds.lowerBound.y = area.position.y;
    bounds.upperBound.x = area.position.x + area.size.x;
    bounds.upperBound.y = area.position.y + area.size.y;
    query_callback = [&callback_function](void* _body)
    {
        return callback_function(static_cast<Simple2DBody*>(_body)->owner);
    };
    broadphase->Query(this, bounds);
    query_callback = nullptr;
}

void Simple2DBackend::queryAny(Ray2d ray, std::function<bool(P<Node> object, Vector2d hit_location, Vector2d hit_normal)> callback_function)
{
    LOG(Warning, "Simple2D raycasting called, but not implemented yet.");
}

void Simple2DBackend::queryAll(Ray2d ray, std::function<bool(P<Node> object, Vector2d hit_location, Vector2d hit_normal)> callback_function)
{
    LOG(Warning, "Simple2D raycasting called, but not implemented yet.");
}

void* Simple2DBackend::createBody(Node* owner, const Simple2DShape& shape)
{
    Simple2DBody* body = new Simple2DBody();
    body->owner = owner;
    body->type = shape.type;
    body->shape = shape.rect;
    body->filter_category = shape.filter_category;
	body->filter_mask = shape.filter_mask;

    body->broadphase_proxy = broadphase->CreateProxy(body->getAABB(), body);

    return body;
}

void Simple2DBackend::AddPair(void* _body_a, void* _body_b)
{
    Simple2DBody* body_a = static_cast<Simple2DBody*>(_body_a);
    Simple2DBody* body_b = static_cast<Simple2DBody*>(_body_b);

    if (!((body_a->filter_category & body_b->filter_mask) && (body_b->filter_category & body_a->filter_mask)))
        return;

    for(auto& pair : collision_pairs)
    {
        if (pair.body_a == body_a && pair.body_b == body_b)
            return;
    }
    collision_pairs.emplace_back();
    collision_pairs.back().body_a = body_a;
    collision_pairs.back().body_b = body_b;
}

bool Simple2DBackend::QueryCallback(int proxy_id)
{
    return query_callback(broadphase->GetUserData(proxy_id));
}

};//namespace collision
};//namespace sp
