#ifndef SP2_SCENE_NODE_H
#define SP2_SCENE_NODE_H

#include <sp2/math/matrix4x4.h>
#include <sp2/math/quaternion.h>
#include <sp2/script/bindingObject.h>
#include <sp2/pointerList.h>
#include <sp2/graphics/scene/renderdata.h>
#include <sp2/graphics/animation.h>
#include <sp2/multiplayer/replication.h>

class b2Body;

namespace sp {
namespace multiplayer {
class Server;
class Client;
}
namespace collision {
class Shape;
class Shape2D;
}
class Node;
class CollisionInfo
{
public:
    P<Node> other;
    float force;
    sp::Vector2d position;
    sp::Vector2d normal;
};
class Scene;
class RenderData;
class Node : public ScriptBindingObject
{
public:
    Node(P<Node> parent);
    virtual ~Node();

    P<Node> getParent();
    P<Scene> getScene();
    PList<Node>& getChildren();
    void setParent(P<Node> new_parent);
    
    void setPosition(sp::Vector2d position);
    void setPosition(sp::Vector3d position);
    void setRotation(double rotation);
    void setRotation(Quaterniond rotation);
    void setLinearVelocity(sp::Vector2d velocity);
    void setLinearVelocity(sp::Vector3d velocity);
    void setAngularVelocity(double velocity);
    void setAngularVelocity(Quaterniond velocity);
    
    sp::Vector2d getPosition2D();
    double getRotation2D();
    sp::Vector2d getLocalPoint2D(sp::Vector2d v);
    
    sp::Vector2d getGlobalPosition2D();
    double getGlobalRotation2D();
    sp::Vector2d getGlobalPoint2D(sp::Vector2d v);
    
    sp::Vector2d getLinearVelocity2D();
    double getAngularVelocity2D();
    
    const Matrix4x4d& getGlobalTransform() const { return global_transform; }
    const Matrix4x4d& getLocalTransform() const { return local_transform; }

    void setCollisionShape(const collision::Shape& shape);
    
    //Test if the given point will collide with the shape of this Node.
    bool testCollision(sp::Vector2d position);
    //Test if this is a solid object, so it has a collision shape, which isn't generated as Sensor.
    bool isSolid();
    
    //Event called every frame.
    //The delta is the time in seconds passed sinds the previous frame, multiplied by the global game speed.
    //Called when the game is paused with delta = 0
    virtual void onUpdate(float delta) {}
    //Event called 30 times per second. Not called when the game is paused.
    virtual void onFixedUpdate() {}
    //Event called when 2 nodes collide. Not called when the game is paused.
    virtual void onCollision(CollisionInfo& info) {}
    
    std::unique_ptr<Animation> animation;
    RenderData render_data;

    class Multiplayer
    {
    public:
        Multiplayer(Node* node);
        ~Multiplayer();
        
        void enable();

        template<typename T> void replicate(T& var)
        {
            replication_links.push_back(new multiplayer::ReplicationLink<T>(&var));
        }
        
        uint64_t getId() const { return id; }
    private:
        Node* node;
        bool enable_replication;
        uint64_t id;
        std::vector<multiplayer::ReplicationLinkBase*> replication_links;
        
        friend class ::sp::multiplayer::Server;
        friend class ::sp::multiplayer::Client;
    } multiplayer;
private:
    Node(Scene* scene);

    P<Scene> scene;
    P<Node> parent;
    PList<Node> children;
    b2Body* collision_body2d;
    
    Vector3d translation;
    Quaterniond rotation;
    
    Matrix4x4d global_transform;
    Matrix4x4d local_transform;
    
    void updateLocalTransform();
    void updateGlobalTransform();
    
    void modifyPositionByPhysics(sp::Vector2d position, double rotation);
    void modifyPositionByPhysics(sp::Vector3d position, Quaterniond rotation);

    friend class Scene;
    friend class collision::Shape2D;
};

};//!namespace sp

#endif//SP2_SCENE_NODE_H
