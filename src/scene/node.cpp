#include <sp2/scene/node.h>
#include <sp2/scene/scene.h>

namespace sp {

SceneNode::SceneNode(P<SceneNode> parent)
: scene(parent->scene), parent(parent)
{
    parent->children.add(this);
}

SceneNode::SceneNode(P<Scene> scene)
: scene(scene)
{
}

P<SceneNode> SceneNode::getParent()
{
    return parent;
}

PVector<SceneNode> SceneNode::getChildren()
{
    return children;
}

void SceneNode::setPosition(sp::Vector2d position)
{
    translation.x = position.x;
    translation.y = position.y;
    updateLocalTransform();
}

void SceneNode::setPosition(sp::Vector3d position)
{
    translation = position;
    updateLocalTransform();
}

void SceneNode::setRotation(Quaterniond rotation)
{
    this->rotation = rotation;
    updateLocalTransform();
}

void SceneNode::setRotation(double rotation)
{
    this->rotation = Quaterniond::fromAngle(rotation);
    updateLocalTransform();
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

};//!namespace sp
