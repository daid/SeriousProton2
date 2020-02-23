#include <sp2/collision/simple2d/simple2dBackend.h>
#include <sp2/collision/simple2d/shape.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/scene/node.h>

#include <private/collision/box2d.h>
#include <private/collision/box2dVector.h>


namespace sp {
namespace collision {

class Simple2DBody;
class CollisionPair
{
public:
    P<Node> node_a;
    P<Node> node_b;
};

class Simple2DBody
{
public:
    Shape::Type type;
    Rect2d rect;
    Node* owner;
    int broadphase_proxy;
    int filter_category;
    int filter_mask;
    
    b2AABB getAABB()
    {
        b2AABB aabb;
        sp::Vector2d position = owner->getPosition2D();
        aabb.lowerBound = toVector<double>(position + rect.position);
        aabb.upperBound = toVector<double>(position + rect.position + rect.size);
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
    for(auto body : delete_list)
    {
        broadphase->DestroyProxy(body->broadphase_proxy);
        delete body;
    }
    delete_list.clear();

    broadphase->UpdatePairs(this);

    collision_pairs.remove_if([this](CollisionPair& pair)
    {
        if (!pair.node_a || !pair.node_b)
            return true;
        Simple2DBody* body_a = static_cast<Simple2DBody*>(getCollisionBody(pair.node_a));
        Simple2DBody* body_b = static_cast<Simple2DBody*>(getCollisionBody(pair.node_b));
        if (!body_a || !body_b)
            return true;
        return !broadphase->TestOverlap(body_a->broadphase_proxy, body_b->broadphase_proxy);
    });
    
    //We make a copy of the collision list, as onCollision could delete an object, which can erase items from the collision pairs list.
    for(auto& pair : collision_pairs)
    {
        if (!pair.node_a || !pair.node_b)
            continue;
        Simple2DBody* body_a = static_cast<Simple2DBody*>(getCollisionBody(pair.node_a));
        Simple2DBody* body_b = static_cast<Simple2DBody*>(getCollisionBody(pair.node_b));
        if (!body_a || !body_b)
            continue;
        Rect2d rect_a = body_a->rect;
        Rect2d rect_b = body_b->rect;
        rect_a.position += pair.node_a->getPosition2D();
        rect_b.position += pair.node_b->getPosition2D();
        if (rect_a.overlaps(rect_b))
        {
            CollisionInfo info;
            
            //Handle static to dynamic object collision, push the dynamic out of the static object.
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
            
            if (body_a->type == Shape::Type::Dynamic && isSolid(body_b))
                modifyPositionByPhysics(body_a->owner, body_a->owner->getPosition2D() + info.normal * double(info.force), 0);
            if (body_b->type == Shape::Type::Dynamic && isSolid(body_a))
                modifyPositionByPhysics(body_b->owner, body_b->owner->getPosition2D() - info.normal * double(info.force), 0);
            
            //Move the points to P<> pointers, as the onCollision could delete one of the objects.
            info.other = pair.node_b;
            pair.node_a->onCollision(info);
            if (pair.node_b)
            {
                info.other = pair.node_a;
                info.normal = -info.normal;
                pair.node_b->onCollision(info);
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
    body->owner = nullptr;

    delete_list.push_back(body);
}

void Simple2DBackend::getDebugRenderMesh(std::vector<std::shared_ptr<MeshData>>& meshes)
{
    b2AABB bounds;
    bounds.lowerBound.x = bounds.lowerBound.y = -std::numeric_limits<float>::infinity();
    bounds.upperBound.x = bounds.upperBound.y = std::numeric_limits<float>::infinity();
    MeshData::Vertices vertices;
    MeshData::Indices indices;
    query_callback = [&vertices, &indices](void* _body)
    {
        Simple2DBody* body = static_cast<Simple2DBody*>(_body);
        if (!body->owner)
            return true;

        Vector2d p0 = body->owner->getPosition2D() + body->rect.position;
        Vector2d p1 = p0 + body->rect.size;
        Vector3f c(0.8, 0.8, 1);
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

    if (meshes.size() < 1)
        meshes.push_back(MeshData::create(std::move(vertices), std::move(indices), MeshData::Type::Dynamic));
    else
        meshes[0]->update(std::move(vertices), std::move(indices));
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
    return body->rect.contains(Vector2d(position.x, position.y) - body->owner->getPosition2D());
}

bool Simple2DBackend::isSolid(void* _body)
{
    Simple2DBody* body = static_cast<Simple2DBody*>(_body);
    return body->type == Shape::Type::Static || body->type == Shape::Type::Kinematic;
}

void Simple2DBackend::query(Vector3d position, std::function<bool(P<Node> object)> callback_function)
{
    b2AABB bounds;
    bounds.lowerBound.x = bounds.upperBound.x = position.x;
    bounds.lowerBound.y = bounds.upperBound.y = position.y;
    query_callback = [&callback_function](void* _body)
    {
        Node* owner = static_cast<Simple2DBody*>(_body)->owner;
        if (owner)
            return callback_function(owner);
        return true;
    };
    broadphase->Query(this, bounds);
    query_callback = nullptr;
}

void Simple2DBackend::query(Vector3d position, double range, std::function<bool(P<Node> object)> callback_function)
{
    b2AABB bounds;
    bounds.lowerBound.x = position.x - range;
    bounds.lowerBound.y = position.y - range;
    bounds.upperBound.x = position.x + range;
    bounds.upperBound.y = position.y + range;
    sp::Vector2d p(position.x, position.y);
    query_callback = [&callback_function, p, range](void* _body)
    {
        Node* owner = static_cast<Simple2DBody*>(_body)->owner;
        if (owner && (owner->getPosition2D() - p).length() <= range)
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
        Node* owner = static_cast<Simple2DBody*>(_body)->owner;
        if (owner)
            return callback_function(owner);
        return true;
    };
    broadphase->Query(this, bounds);
    query_callback = nullptr;
}

void Simple2DBackend::queryAny(Ray3d ray, std::function<bool(P<Node> object, Vector3d hit_location, Vector3d hit_normal)> callback_function)
{
    LOG(Warning, "Simple2D raycasting called, but not implemented yet.");
}

void Simple2DBackend::queryAll(Ray3d ray, std::function<bool(P<Node> object, Vector3d hit_location, Vector3d hit_normal)> callback_function)
{
    LOG(Warning, "Simple2D raycasting called, but not implemented yet.");
}

void* Simple2DBackend::createBody(Node* owner, const Simple2DShape& shape)
{
    Simple2DBody* body = new Simple2DBody();
    body->owner = owner;
    body->type = shape.type;
    body->rect = shape.rect;
    body->rect.position -= body->rect.size / 2.0;
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
        if (pair.node_a == body_a->owner && pair.node_b == body_b->owner)
            return;
    }
    collision_pairs.emplace_back();
    collision_pairs.back().node_a = body_a->owner;
    collision_pairs.back().node_b = body_b->owner;
}

bool Simple2DBackend::QueryCallback(int proxy_id)
{
    return query_callback(broadphase->GetUserData(proxy_id));
}

}//namespace collision
}//namespace sp
