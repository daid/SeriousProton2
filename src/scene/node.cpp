#include <sp2/scene/node.h>
#include <sp2/scene/scene.h>
#include <sp2/collision/shape.h>
#include <sp2/logging.h>
#include <sp2/assert.h>
#include <sp2/multiplayer/server.h>
#include <sp2/multiplayer/registry.h>
#include <Box2D/Box2D.h>
#include <btBulletDynamicsCommon.h>
#include <private/collision/box2dVector.h>
#include <private/collision/bulletVector.h>
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
    
    local_transform = Matrix4x4f::identity();
    updateGlobalTransform();
}

Node::Node(Scene* scene)
: multiplayer(this), scene(scene)
{
    collision_body = nullptr;
    parent = nullptr;
    
    global_transform = Matrix4x4f::identity();
    local_transform = Matrix4x4f::identity();
}

Node::~Node()
{
    if (collision_body)
        scene->collision_backend->destroyBody(collision_body);
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
    sp2assert(!collision_body, "Tried to switch parent of node that has collision attached. This is not supported.");
    sp2assert(parent, "Tried to switch parent of root node. This is not supported.");
    sp2assert(new_parent, "When switching parents, you must provide a new parent, not nullptr.");
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
    if (collision_body)
        scene->collision_backend->updatePosition(collision_body, sp::Vector3d(position.x, position.y, 0));
    updateLocalTransform();
}

void Node::setPosition(sp::Vector3d position)
{
    translation = position;
    if (collision_body)
        scene->collision_backend->updatePosition(collision_body, position);
    updateLocalTransform();
}

void Node::setRotation(double rotation)
{
    this->rotation = Quaterniond::fromAngle(rotation);
    if (collision_body)
        scene->collision_backend->updateRotation(collision_body, rotation);
    updateLocalTransform();
}

void Node::setRotation(Quaterniond rotation)
{
    this->rotation = rotation;
    this->rotation.normalize();
    updateLocalTransform();
}

void Node::setLinearVelocity(sp::Vector2d velocity)
{
    if (collision_body)
        scene->collision_backend->setLinearVelocity(collision_body, sp::Vector3d(velocity.x, velocity.y, 0));
}

void Node::setLinearVelocity(Vector3d velocity)
{
    if (collision_body)
        scene->collision_backend->setLinearVelocity(collision_body, velocity);
}

void Node::setAngularVelocity(double velocity)
{
    if (collision_body)
        scene->collision_backend->setAngularVelocity(collision_body, sp::Vector3d(0, 0, velocity));
}

void Node::setAngularVelocity(sp::Vector3d velocity)
{
    if (collision_body)
        scene->collision_backend->setAngularVelocity(collision_body, velocity);
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
    return Vector2d(global_transform * Vector2f(0, 0));
}

double Node::getGlobalRotation2D()
{
    return global_transform.applyDirection(Vector2f(1, 0)).angle();
}

Vector2d Node::getLocalPoint2D(Vector2d v)
{
    return Vector2d(local_transform * Vector2f(v));
}

Vector2d Node::getGlobalPoint2D(Vector2d v)
{
    return Vector2d(global_transform * Vector2f(v));
}

sp::Vector2d Node::getLinearVelocity2D()
{
    if (collision_body)
    {
        Vector3d velocity = scene->collision_backend->getLinearVelocity(collision_body);
        return Vector2d(velocity.x, velocity.y);
    }
    return Vector2d(0.0, 0.0);
}

double Node::getAngularVelocity2D()
{
    if (collision_body)
    {
        Vector3d velocity = scene->collision_backend->getAngularVelocity(collision_body);
        return velocity.z;
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

Vector3d Node::getGlobalPosition3D()
{
    return Vector3d(global_transform * Vector3f(0, 0, 0));
}

Vector3d Node::getGlobalPoint3D(Vector3d v)
{
    return Vector3d(global_transform * Vector3f(v));
}

Vector3d Node::getLinearVelocity3D()
{
    if (collision_body)
        return scene->collision_backend->getLinearVelocity(collision_body);
    return Vector3d(0, 0, 0);
}

Vector3d Node::getAngularVelocity3D()
{
    if (collision_body)
        return scene->collision_backend->getAngularVelocity(collision_body);
    return Vector3d(0, 0, 0);
}

void Node::setCollisionShape(const collision::Shape& shape)
{
    shape.create(this);
}

void Node::removeCollisionShape()
{
    if (collision_body)
        scene->collision_backend->destroyBody(collision_body);
    collision_body = nullptr;
}

bool Node::testCollision(sp::Vector2d position)
{
    if (!collision_body)
        return false;
    return scene->collision_backend->testCollision(collision_body, Vector3d(position.x, position.y, 0));
}

bool Node::isSolid()
{
    if (!collision_body)
        return false;
    return scene->collision_backend->isSolid(collision_body);
}

void Node::setAnimation(std::unique_ptr<Animation> animation)
{
    this->animation = std::move(animation);
    if (this->animation)
        this->animation->prepare(render_data);
}

void Node::animationPlay(string key, float speed)
{
    if (animation)
        animation->play(key, speed);
}

void Node::animationSetFlags(int flags)
{
    if (animation)
        animation->setFlags(flags);
}

int Node::animationGetFlags()
{
    if (animation)
        return animation->getFlags();
    return 0;
}

void Node::updateLocalTransform()
{
    local_transform = Matrix4x4f::translate(Vector3f(translation)) * Matrix4x4f::fromQuaternion(Quaternionf(rotation));
    updateGlobalTransform();
}

void Node::updateGlobalTransform()
{
    if (parent && parent != scene->getRoot())
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
