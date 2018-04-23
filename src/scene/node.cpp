#include <sp2/scene/node.h>
#include <sp2/scene/scene.h>
#include <sp2/collision/shape.h>
#include <sp2/logging.h>
#include <sp2/assert.h>
#include <sp2/multiplayer/server.h>
#include <sp2/multiplayer/registry.h>
#include <Box2D/Box2D.h>
#include <private/collision/box2dVector.h>
#include <cmath>
#include <typeindex>

namespace sp {

REGISTER_MULTIPLAYER_CLASS(Node);

Node::Node(P<Node> parent)
: multiplayer(this), parent(parent)
{
    sp2assert(parent != nullptr, "Tried to create Node without a parent.");
    
    scene = parent->scene;
    parent->children.add(this);
    collision_body2d = nullptr;
    
    local_transform = Matrix4x4d::identity();
    updateGlobalTransform();
}

Node::Node(Scene* scene)
: multiplayer(this), scene(scene)
{
    collision_body2d = nullptr;
    parent = nullptr;
    
    global_transform = Matrix4x4d::identity();
    local_transform = Matrix4x4d::identity();
}

Node::~Node()
{
    if (collision_body2d)
        scene->destroyCollisionBody2D(collision_body2d);
    for(Node* child : children)
        delete child;
}

P<Node> Node::getParent()
{
    return parent;
}

P<Scene> Node::getScene()
{
    return scene;
}

const PList<Node>& Node::getChildren()
{
    return children;
}

void Node::setParent(P<Node> new_parent)
{
    sp2assert(!collision_body2d, "Tried to switch parent of node that has collision attached. This is not supported.");
    sp2assert(!multiplayer.isEnabled(), "Tried to switch parents on a multiplayer enabled node. This is not supported.");
    
    parent->children.remove(P<Node>(this));
    parent = new_parent;
    parent->children.add(this);

    if (new_parent->scene != scene)
    {
        scene = new_parent->scene;
        for(Node* child : children)
        {
            sp2assert(child->children.size() == 0, "Implementation incomplete...");
            child->scene = new_parent->scene;
        }
    }
    
    updateGlobalTransform();
}

void Node::setPosition(sp::Vector2d position)
{
    translation.x = position.x;
    translation.y = position.y;
    if (collision_body2d)
    {
        collision_body2d->SetTransform(toVector(position), collision_body2d->GetAngle());
    }
    updateLocalTransform();
}

void Node::setPosition(sp::Vector3d position)
{
    translation = position;
    if (collision_body2d)
    {
        collision_body2d->SetTransform(toVector(sp::Vector2d(translation.x, translation.y)), collision_body2d->GetAngle());
    }
    updateLocalTransform();
}

void Node::setRotation(double rotation)
{
    this->rotation = Quaterniond::fromAngle(rotation);
    if (collision_body2d)
    {
        collision_body2d->SetTransform(collision_body2d->GetPosition(), rotation / 180.0 * pi);
    }
    updateLocalTransform();
}

void Node::setRotation(Quaterniond rotation)
{
    this->rotation = rotation;
    if (collision_body2d)
    {
        //TODO: Update collision body rotation
        //collision_body2d->SetTransform(collision_body2d->GetPosition(), rotation / 180.0 * pi);
    }
    updateLocalTransform();
}

void Node::setLinearVelocity(sp::Vector2d velocity)
{
    if (collision_body2d)
    {
        collision_body2d->SetLinearVelocity(toVector(velocity));
    }
}

void Node::setAngularVelocity(double velocity)
{
    if (collision_body2d)
        collision_body2d->SetAngularVelocity(velocity / 180.0 * pi);
}

Vector2d Node::getPosition2D()
{
    return Vector2d(translation.x, translation.y);
}

double Node::getRotation2D()
{
    return (rotation * Vector2d(1, 0)).angle();
}

Vector2d Node::getGlobalPosition2D()
{
    return global_transform * Vector2d(0, 0);
}

double Node::getGlobalRotation2D()
{
    return global_transform.applyDirection(Vector2d(1, 0)).angle();
}

Vector2d Node::getLocalPoint2D(Vector2d v)
{
    return local_transform * v;
}

sp::Vector2d Node::getGlobalPoint2D(sp::Vector2d v)
{
    return global_transform * v;
}

sp::Vector2d Node::getLinearVelocity2D()
{
    if (collision_body2d)
    {
        return toVector<double>(collision_body2d->GetLinearVelocity());
    }
    return sp::Vector2d(0.0, 0.0);
}

double Node::getAngularVelocity2D()
{
    if (collision_body2d)
    {
        return collision_body2d->GetAngularVelocity() / pi * 180.0f;
    }
    return 0.0;
}

Vector3d Node::getPosition3D()
{
    return translation;
}

Quaterniond Node::getRotation3D()
{
    return rotation;
}

void Node::setCollisionShape(const collision::Shape& shape)
{
    shape.create(this);
}

void Node::removeCollisionShape()
{
    if (collision_body2d)
        scene->destroyCollisionBody2D(collision_body2d);
    collision_body2d = nullptr;
}

bool Node::testCollision(sp::Vector2d position)
{
    if (!collision_body2d)
        return false;
    for(const b2Fixture* f = collision_body2d->GetFixtureList(); f; f = f->GetNext())
    {
        if (f->TestPoint(toVector(position)))
            return true;
    }
    return false;
}

bool Node::isSolid()
{
    if (!collision_body2d)
        return false;
    for(const b2Fixture* f = collision_body2d->GetFixtureList(); f; f = f->GetNext())
    {
        if (!f->IsSensor())
            return true;
    }
    return false;
}

void Node::updateLocalTransform()
{
    local_transform = Matrix4x4d::translate(translation) * Matrix4x4d::fromQuaternion(rotation);
    updateGlobalTransform();
}

void Node::updateGlobalTransform()
{
    if (parent)
        global_transform = parent->global_transform * local_transform;
    else
        global_transform = local_transform;

    for(P<Node> n : children)
        n->updateGlobalTransform();
}

void Node::modifyPositionByPhysics(sp::Vector2d position, double rotation)
{
    translation.x = position.x;
    translation.y = position.y;
    this->rotation = Quaterniond::fromAngle(rotation);
    updateLocalTransform();
}

void Node::modifyPositionByPhysics(sp::Vector3d position, Quaterniond rotation)
{
    translation = position;
    this->rotation = rotation;
    updateLocalTransform();
}

Node::Multiplayer::Multiplayer(Node* node)
: node(node)
{
    enabled = false;
    id = 0;
}

Node::Multiplayer::~Multiplayer()
{
    for(auto link : replication_links)
        delete link;
}

void Node::Multiplayer::enable()
{
    if (enabled)
        return;
    if (node->parent)
        sp2assert(node->parent->multiplayer.enabled, "Parent of a multiplayer enabled object should also be multiplayer enabled.");
    enabled = true;
}

bool Node::Multiplayer::isEnabled()
{
    return enabled;
}

};//namespace sp
