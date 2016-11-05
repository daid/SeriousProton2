#include <sp2/scene/node.h>
#include <sp2/scene/scene.h>
#include <sp2/collision/shape.h>
#include <sp2/logging.h>
#include <Box2D/Box2D.h>
#include <private/collision/box2dVector.h>
#include <cmath>

namespace sp {

SceneNode::SceneNode(P<SceneNode> parent)
: scene(parent->scene), parent(parent)
{
    parent->children.add(this);
    collision_body2d = nullptr;
    
    global_transform = Matrix4x4d::identity();
    local_transform = Matrix4x4d::identity();
}

SceneNode::SceneNode(Scene* scene)
: scene(scene)
{
    collision_body2d = nullptr;
    
    global_transform = Matrix4x4d::identity();
    local_transform = Matrix4x4d::identity();
}

SceneNode::~SceneNode()
{
    if (collision_body2d)
    {
        scene->destroyCollisionBody2D(collision_body2d);
    }
}

P<SceneNode> SceneNode::getParent()
{
    return parent;
}

P<Scene> SceneNode::getScene()
{
    return scene;
}

PVector<SceneNode> SceneNode::getChildren()
{
    return children;
}

void SceneNode::setPosition(sp::Vector2d position)
{
    translation.x = position.x;
    translation.y = position.y;
    if (collision_body2d)
    {
        collision_body2d->SetTransform(toVector(position), collision_body2d->GetAngle());
    }
    updateLocalTransform();
}

void SceneNode::setPosition(sp::Vector3d position)
{
    translation = position;
    if (collision_body2d)
    {
        collision_body2d->SetTransform(toVector(sp::Vector2d(translation.x, translation.y)), collision_body2d->GetAngle());
    }
    updateLocalTransform();
}

void SceneNode::setRotation(double rotation)
{
    this->rotation = Quaterniond::fromAngle(rotation);
    if (collision_body2d)
    {
        collision_body2d->SetTransform(collision_body2d->GetPosition(), rotation / 180.0 * pi);
    }
    updateLocalTransform();
}

void SceneNode::setRotation(Quaterniond rotation)
{
    this->rotation = rotation;
    if (collision_body2d)
    {
        //TODO: Update collision body rotation
        //collision_body2d->SetTransform(collision_body2d->GetPosition(), rotation / 180.0 * pi);
    }
    updateLocalTransform();
}

void SceneNode::setVelocity(sp::Vector2d velocity)
{
    if (collision_body2d)
    {
        collision_body2d->SetLinearVelocity(toVector(velocity));
    }
}

void SceneNode::setVelocity(sp::Vector3d velocity)
{
    //TODO
}

sp::Vector2d SceneNode::getLocalPosition2D()
{
    return sp::Vector2d(translation.x, translation.y);
}

double SceneNode::getLocalRotation2D()
{
    sp::Vector2d v = rotation * sp::Vector2d(1, 0);
    return std::atan2(v.y, v.x) / pi * 180.0f;
}

sp::Vector2d SceneNode::getGlobalPosition2D()
{
    return global_transform * sp::Vector2d(0, 0);
}

double SceneNode::getGlobalRotation2D()
{
    sp::Vector2d v = global_transform.applyDirection(sp::Vector2d(1, 0));
    return std::atan2(v.y, v.x) / pi * 180.0f;
}

sp::Vector2d SceneNode::getGlobalVelocity2D()
{
    if (collision_body2d)
    {
        return toVector<double>(collision_body2d->GetLinearVelocity());
    }
    return sp::Vector2d(0.0, 0.0);
}

void SceneNode::setCollisionShape(const collision::Shape& shape)
{
    shape.create(this);
}

void SceneNode::updateLocalTransform()
{
    local_transform = Matrix4x4d::translate(translation) * Matrix4x4d::fromQuaternion(rotation);
    updateGlobalTransform();
}

void SceneNode::updateGlobalTransform()
{
    if (parent)
        global_transform = parent->global_transform * local_transform;
    else
        global_transform = local_transform;

    for(P<SceneNode> n : children)
        n->updateGlobalTransform();
}

void SceneNode::modifyPositionByPhysics(sp::Vector2d position, double rotation)
{
    translation.x = position.x;
    translation.y = position.y;
    this->rotation = Quaterniond::fromAngle(rotation);
    updateLocalTransform();
}

void SceneNode::modifyPositionByPhysics(sp::Vector3d position, Quaterniond rotation)
{
    translation = position;
    this->rotation = rotation;
    updateLocalTransform();
}

};//!namespace sp
