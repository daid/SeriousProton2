#ifndef SP2_SCENE_NODE_H
#define SP2_SCENE_NODE_H

#include <sp2/math/matrix4x4.h>
#include <sp2/math/quaternion.h>
#include <sp2/script/bindingObject.h>
#include <sp2/pointerVector.h>

namespace sp {

class Scene;
class SceneNode : public ScriptBindingObject
{
public:
    SceneNode(P<SceneNode> parent);

    P<SceneNode> getParent();
    PVector<SceneNode> getChildren();
    
    void setPosition(sp::Vector2d position);
    void setPosition(sp::Vector3d position);
    void setRotation(Quaterniond rotation);
    void setRotation(double rotation);
private:
    SceneNode(P<Scene> scene);

    P<Scene> scene;
    P<SceneNode> parent;
    PVector<SceneNode> children;
    
    Vector3d translation;
    Quaterniond rotation;
    
    Matrix4x4d global_transform;
    Matrix4x4d local_transform;
    
    void updateLocalTransform();
    void updateGlobalTransform();
    
    friend class Scene;
};

};//!namespace sp

#endif//SP2_SCENE_NODE_H
