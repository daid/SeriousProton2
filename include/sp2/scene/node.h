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
class btRigidBody;

namespace sp {
namespace multiplayer {
class Server;
class Client;
}
namespace collision {
class Backend;
class Shape;
class Joint2D;
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
    const PList<Node>& getChildren();
    void setParent(P<Node> new_parent);
    
    void setPosition(Vector2d position);
    void setPosition(Vector3d position);
    void setRotation(double rotation);
    void setRotation(Quaterniond rotation);
    void setLinearVelocity(Vector2d velocity);
    void setLinearVelocity(Vector3d velocity);
    void setAngularVelocity(double velocity);
    void setAngularVelocity(Vector3d velocity);
    
    Vector2d getPosition2D();
    double getRotation2D();
    Vector2d getLocalPoint2D(Vector2d v);
    
    Vector2d getGlobalPosition2D();
    double getGlobalRotation2D();
    Vector2d getGlobalPoint2D(Vector2d v);
    
    Vector2d getLinearVelocity2D();
    double getAngularVelocity2D();

    Vector3d getPosition3D();
    Quaterniond getRotation3D();

    Vector3d getGlobalPosition3D();
    Vector3d getGlobalPoint3D(Vector3d v);

    Vector3d getLinearVelocity3D();
    Vector3d getAngularVelocity3D();
    
    const Matrix4x4f& getGlobalTransform() const { return global_transform; }
    const Matrix4x4f& getLocalTransform() const { return local_transform; }

    //Set or replace the current collision shape on this body.
    //If you want to shape change, you do not need to call removeCollisionShape() before calling setCollisionShape (doing so will reset the velocity)
    void setCollisionShape(const collision::Shape& shape);
    //Remove the collision shape.
    void removeCollisionShape();
    
    //Test if the given point will collide with the shape of this Node.
    bool testCollision(sp::Vector2d position);
    //Test if this is a solid object, so it has a collision shape, which isn't generated as Sensor.
    bool isSolid();
    
    void setAnimation(std::unique_ptr<Animation> animation);
    void animationPlay(string key, float speed=1.0);
    void animationSetFlags(int flags);
    int animationGetFlags();
    bool animationIsFinished();
    
    //Event called every frame.
    //The delta is the time in seconds passed sinds the previous frame, multiplied by the global game speed.
    //Called when the game is paused with delta = 0
    virtual void onUpdate(float delta) {}
    //Event called 30 times per second. Not called when the game is paused.
    virtual void onFixedUpdate() {}
    //Event called when 2 nodes collide. Not called when the game is paused.
    virtual void onCollision(CollisionInfo& info) {}
    
    RenderData render_data;

    class Multiplayer
    {
    public:
        Multiplayer(Node* node);
        ~Multiplayer();
        
        void enable();
        bool isEnabled();

        void enableDeadReckoning(const multiplayer::DeadReckoningConfig& config)
        {
            replication_links.push_back(new multiplayer::ReplicationDeadReckoning(*node, config));
        }
        
        template<typename T> void replicate(T& var)
        {
            replication_links.push_back(new multiplayer::ReplicationLink<T>(&var));
        }

        template<typename T> void replicate(T& var, float max_update_interval)
        {
            replication_links.push_back(new multiplayer::ReplicationLink<T>(&var, max_update_interval));
        }

        template<typename CLASS, typename... ARGS> void replicate(void(CLASS::*func)(ARGS...))
        {
            replication_calls.push_back(new multiplayer::ReplicationCallInfo<CLASS, ARGS...>(func));
        }
        
        template<typename CLASS, typename... ARGS> void callOnServer(void(CLASS::*func)(ARGS...), ARGS... args)
        {
            for(unsigned int n=0; n<replication_calls.size(); n++)
            {
                if (replication_calls[n]->getPtr() == multiplayer::ReplicationCallInfoBase::ReplicationCallInfoBase::BaseFuncPtr(func))
                {
                    prepared_calls.emplace_back(uint16_t(n), args...);
                    return;
                }
            }
        }
        
        uint64_t getId() const
        {
            return id;
        }
    private:
        Node* node;
        bool enabled;
        uint64_t id;
        std::vector<multiplayer::ReplicationLinkBase*> replication_links;
        std::vector<multiplayer::ReplicationCallInfoBase*> replication_calls;
        std::vector<io::DataBuffer> prepared_calls;
        
        friend class ::sp::multiplayer::Server;
        friend class ::sp::multiplayer::Client;
    } multiplayer;
private:
    Node(Scene* scene);

    P<Scene> scene;
    P<Node> parent;
    PList<Node> children;
    void* collision_body = nullptr;
    
    Vector3d translation;
    Quaterniond rotation;
    
    Matrix4x4f global_transform;
    Matrix4x4f local_transform;

    std::unique_ptr<Animation> animation;
    
    void updateLocalTransform();
    void updateGlobalTransform();
    
    void modifyPositionByPhysics(sp::Vector2d position, double rotation);
    void modifyPositionByPhysics(sp::Vector3d position, Quaterniond rotation);

    friend class Scene;
    friend class collision::Shape;
    friend class collision::Backend;
    friend class collision::Joint2D;
};

};//namespace sp

#endif//SP2_SCENE_NODE_H
