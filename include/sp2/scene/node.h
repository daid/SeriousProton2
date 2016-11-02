#ifndef SP2_SCENE_NODE_H
#define SP2_SCENE_NODE_H

#include <sp2/math/matrix4x4.h>
#include <sp2/math/quaternion.h>
#include <sp2/script/bindingObject.h>
#include <sp2/pointerVector.h>
#include <sp2/graphics/scene/renderdata.h>

class b2Body;

namespace sp {
namespace collision {
class Shape;
class Shape2D;
}
class Scene;
class RenderData;
class SceneNode : public ScriptBindingObject
{
public:
    SceneNode(P<SceneNode> parent);

    P<SceneNode> getParent();
    P<Scene> getScene();
    PVector<SceneNode> getChildren();
    
    void setPosition(sp::Vector2d position);
    void setPosition(sp::Vector3d position);
    void setRotation(Quaterniond rotation);
    void setRotation(double rotation);
    
    sp::Vector2d getLocalPosition2D();
    double getLocalRotation2D();
    sp::Vector2d getGlobalPosition2D();
    double getGlobalRotation2D();
    
    const Matrix4x4d& getGlobalTransform() const { return global_transform; }
    
    void setCollisionShape(const collision::Shape& shape);
    
    //Event called every frame.
    //The delta is the time in seconds passed sinds the previous frame, multiplied by the global game speed.
    //Called when the game is paused with delta = 0
    virtual void onUpdate(float delta) {}
    //Event called 30 times per second. Not called when the game is paused.
    virtual void onFixedUpdate() {}
    
    RenderData render_data;
private:
    SceneNode(Scene* scene);

    P<Scene> scene;
    P<SceneNode> parent;
    PVector<SceneNode> children;
    b2Body* collision_body2d;
    
    Vector3d translation;
    Quaterniond rotation;
    
    Matrix4x4d global_transform;
    Matrix4x4d local_transform;
    
    void updateLocalTransform();
    void updateGlobalTransform();
    
    friend class Scene;
    friend class collision::Shape2D;
};

};//!namespace sp

#endif//SP2_SCENE_NODE_H
