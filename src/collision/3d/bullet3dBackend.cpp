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
    Collision3DDebugRender(std::vector<std::shared_ptr<MeshData>>& meshes)
    : meshes(meshes)
    {
    }

    ~Collision3DDebugRender()
    {
        if (indices.size() > 0)
            addToMeshes();
        meshes.resize(mesh_index);
    }

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

        if (index > 65000)
            addToMeshes();
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

private:
    void addToMeshes()
    {
        if (mesh_index < meshes.size())
            meshes[mesh_index]->update(std::move(vertices), std::move(indices));
        else
            meshes.push_back(MeshData::create(std::move(vertices), std::move(indices), MeshData::Type::Dynamic));
        mesh_index++;
    }

    size_t mesh_index = 0;
    std::vector<std::shared_ptr<MeshData>>& meshes;
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
        Node* node = static_cast<Node*>(obj->getUserPointer());
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

void BulletBackend::getDebugRenderMesh(std::vector<std::shared_ptr<MeshData>>& meshes)
{
    Collision3DDebugRender debug_renderer(meshes);
    
    world->setDebugDrawer(&debug_renderer);
    world->debugDrawWorld();
    world->setDebugDrawer(nullptr);
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
    LOG(Warning, "Bullet3D testCollision called, but not implemented yet.");
    return false;
}

bool BulletBackend::isSolid(void* _body)
{
    btRigidBody* body = static_cast<btRigidBody*>(_body);
    return !(body->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE);
}

void BulletBackend::query(sp::Vector3d position, std::function<bool(P<Node> object)> callback_function)
{
    LOG(Warning, "Bullet3D query called, but not implemented yet.");
}

void BulletBackend::query(sp::Vector3d position, double range, std::function<bool(P<Node> object)> callback_function)
{
    LOG(Warning, "Bullet3D query called, but not implemented yet.");
}

void BulletBackend::query(Rect2d area, std::function<bool(P<Node> object)> callback_function)
{
    LOG(Warning, "Bullet3D query called, but not implemented yet.");
}

class BulletRaycastCallback : public btCollisionWorld::RayResultCallback
{
public:
    BulletRaycastCallback()
    {
    }

    btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace) override
    {
        sp::P<Node> node = static_cast<Node*>(rayResult.m_collisionObject->getUserPointer());
        entries.emplace_back(node, rayResult.m_hitFraction, toVector<double>(rayResult.m_hitNormalLocal));
        return 1.0f;
    }

    struct Entry
    {
        Entry(sp::P<Node> node, float fraction, sp::Vector3d normal): node(node), fraction(fraction), normal(normal) {}
        sp::P<Node> node;
        float fraction;
        sp::Vector3d normal;
    };
    std::vector<Entry> entries;
};

void BulletBackend::queryAny(Ray3d ray, std::function<bool(P<Node> object, Vector3d hit_location, Vector3d hit_normal)> callback_function)
{
    BulletRaycastCallback callback_object;
    world->rayTest(toVector(ray.start), toVector(ray.end), callback_object);
    for(auto entry : callback_object.entries)
    {
        if (entry.node)
        {
            if (!callback_function(entry.node, ray.start + (ray.end - ray.start) * double(entry.fraction), entry.normal))
                return;
        }
    }
}

void BulletBackend::queryAll(Ray3d ray, std::function<bool(P<Node> object, Vector3d hit_location, Vector3d hit_normal)> callback_function)
{
    BulletRaycastCallback callback_object;
    world->rayTest(toVector(ray.start), toVector(ray.end), callback_object);
    std::sort(callback_object.entries.begin(), callback_object.entries.end(), [](const BulletRaycastCallback::Entry& a, const BulletRaycastCallback::Entry& b)
    {
        return a.fraction < b.fraction;
    });
    for(auto entry : callback_object.entries)
    {
        if (entry.node)
        {
            if (!callback_function(entry.node, ray.start + (ray.end - ray.start) * double(entry.fraction), entry.normal))
                return;
        }
    }
}

}//namespace collision
}//namespace sp
