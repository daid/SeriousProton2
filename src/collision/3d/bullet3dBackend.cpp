#include <sp2/collision/3d/bullet3dBackend.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/scene/node.h>

#include <private/collision/bulletVector.h>
#include <private/collision/bullet.h>


namespace sp {
namespace collision {

class Collision3DDebugRender : public btIDebugDraw
{
public:
    virtual void drawLine(const btVector3& from,const btVector3& to,const btVector3& color) override
    {
        Vector3f c = toVector<float>(color);

        Vector3f v0 = toVector<float>(from);
        Vector3f v1 = toVector<float>(to);
        Vector3f diff = (v1 - v0);
        
        //This isn't perfect, but it works good enough.
        diff = diff.cross(sp::Vector3f(0, 0, 1)).normalized() * 0.2f;
        
        int index = vertices.size();
        vertices.emplace_back(v0, c, Vector2f());
        vertices.emplace_back(v1, c, Vector2f());
        vertices.emplace_back((v0 + v1) * 0.5f - diff, c, Vector2f());
        vertices.emplace_back((v0 + v1) * 0.5f + diff, c, Vector2f());

        indices.emplace_back(index);
        indices.emplace_back(index + 1);
        indices.emplace_back(index + 2);
        indices.emplace_back(index);
        indices.emplace_back(index + 1);
        indices.emplace_back(index + 3);
    }

    virtual void drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color) override
    {
    }

	virtual void reportErrorWarning(const char* warningString) override
	{
	}

	virtual void draw3dText(const btVector3& location,const char* textString) override
	{
	}
	
	virtual void setDebugMode(int debugMode) override
	{
	}
	
	virtual int getDebugMode() const override
	{
        return DBG_DrawWireframe;
	}

    sp::MeshData::Vertices vertices;
    sp::MeshData::Indices indices;
};

BulletBackend::BulletBackend()
{
    configuration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(configuration);
    broadphase = new btDbvtBroadphase();
    solver = new btSequentialImpulseConstraintSolver();
    world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, configuration);
    world->setGravity(btVector3(0, 0, 0));
}

BulletBackend::~BulletBackend()
{
    delete world;
    delete solver;
    delete broadphase;
    delete dispatcher;
    delete configuration;
}

void BulletBackend::step(float time_delta)
{
    world->stepSimulation(time_delta);
        //TODO: Collisions events
        /*
    int numManifolds = world->getDispatcher()->getNumManifolds();
    for (int i = 0; i < numManifolds; i++)
    {
        btPersistentManifold* contactManifold =  world->getDispatcher()->getManifoldByIndexInternal(i);
        const btCollisionObject* obA = contactManifold->getBody0();
        const btCollisionObject* obB = contactManifold->getBody1();

        int numContacts = contactManifold->getNumContacts();
        for (int j = 0; j < numContacts; j++)
        {
            btManifoldPoint& pt = contactManifold->getContactPoint(j);
            if (pt.getDistance() < 0.f)
            {
                const btVector3& ptA = pt.getPositionWorldOnA();
                const btVector3& ptB = pt.getPositionWorldOnB();
                const btVector3& normalOnB = pt.m_normalWorldOnB;
            }
        }
    }        */
}

void BulletBackend::postUpdate(float delta)
{
    for(int index=0; index<world->getNumCollisionObjects(); index++)
    {
        btCollisionObject* obj = world->getCollisionObjectArray()[index];
        Node* node = (Node*)obj->getUserPointer();
        btTransform transform = obj->getWorldTransform();
        modifyPositionByPhysics(node, toVector<double>(transform.getOrigin()), toQuadernion<double>(transform.getRotation()));
    }
}

void BulletBackend::destroyBody(void* _body)
{
    btRigidBody* body = static_cast<btRigidBody*>(_body);
    world->removeCollisionObject(body);
    btCollisionShape* shape = body->getCollisionShape();
    delete body;
    delete shape;
}

void BulletBackend::getDebugRenderMesh(std::shared_ptr<MeshData>& mesh)
{
    Collision3DDebugRender debug_renderer;
    
    world->setDebugDrawer(&debug_renderer);
    world->debugDrawWorld();
    world->setDebugDrawer(nullptr);

    if (!mesh)
        mesh = MeshData::create(std::move(debug_renderer.vertices), std::move(debug_renderer.indices), MeshData::Type::Dynamic);
    else
        mesh->update(std::move(debug_renderer.vertices), std::move(debug_renderer.indices));
}

void BulletBackend::updatePosition(void* _body, sp::Vector3d position)
{
    btRigidBody* body = static_cast<btRigidBody*>(_body);
    body->getWorldTransform().setOrigin(toVector(position));
    body->activate();
}

void BulletBackend::updateRotation(void* _body, float rotation)
{
    btRigidBody* body = static_cast<btRigidBody*>(_body);
    body->getWorldTransform().setRotation(toQuadernion(Quaterniond::fromAngle(rotation)));
}

void BulletBackend::updateRotation(void* _body, Quaterniond rotation)
{
    btRigidBody* body = static_cast<btRigidBody*>(_body);
    body->getWorldTransform().setRotation(toQuadernion(rotation));
}

void BulletBackend::setLinearVelocity(void* _body, Vector3d velocity)
{
    btRigidBody* body = static_cast<btRigidBody*>(_body);
    body->setLinearVelocity(toVector(velocity));
    body->activate();
}

void BulletBackend::setAngularVelocity(void* _body, Vector3d velocity)
{
    btRigidBody* body = static_cast<btRigidBody*>(_body);
    body->setAngularVelocity(toVector(velocity / 180.0 * pi));
    body->activate();
}

Vector3d BulletBackend::getLinearVelocity(void* _body)
{
    btRigidBody* body = static_cast<btRigidBody*>(_body);
    return toVector<double>(body->getLinearVelocity());
}

Vector3d BulletBackend::getAngularVelocity(void* _body)
{
    btRigidBody* body = static_cast<btRigidBody*>(_body);
    return toVector<double>(body->getAngularVelocity()) / pi * 180.0;
}

bool BulletBackend::testCollision(void* _body, Vector3d position)
{
    //btRigidBody* body = static_cast<btRigidBody*>(_body);
    return false;
}

bool BulletBackend::isSolid(void* _body)
{
    btRigidBody* body = static_cast<btRigidBody*>(_body);
    return !(body->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE);
}

void BulletBackend::query(sp::Vector2d position, std::function<bool(P<Node> object)> callback_function)
{
}

void BulletBackend::query(sp::Vector2d position, double range, std::function<bool(P<Node> object)> callback_function)
{
}

void BulletBackend::query(Rect2d area, std::function<bool(P<Node> object)> callback_function)
{
}

void BulletBackend::queryAny(Ray2d ray, std::function<bool(P<Node> object, Vector2d hit_location, Vector2d hit_normal)> callback_function)
{
}

void BulletBackend::queryAll(Ray2d ray, std::function<bool(P<Node> object, Vector2d hit_location, Vector2d hit_normal)> callback_function)
{
}

};//namespace collision
};//namespace sp
