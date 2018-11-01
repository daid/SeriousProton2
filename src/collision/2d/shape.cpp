#include <sp2/collision/2d/shape.h>
#include <sp2/collision/2d/joint.h>
#include <sp2/scene/node.h>
#include <sp2/scene/scene.h>
#include <sp2/assert.h>

#include <private/collision/box2dVector.h>
#include <Box2D/Box2D.h>

namespace sp {
namespace collision {

class ContactListener : public b2ContactListener
{
public:
	virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
	{
		b2Fixture* fixture_a = contact->GetFixtureA();
		b2Fixture* fixture_b = contact->GetFixtureB();

        if (fixture_a->GetType() == b2Shape::Type::e_chain)
        {
            b2ChainShape* chain = (b2ChainShape*)fixture_a->GetShape();
            b2EdgeShape edge;
            chain->GetChildEdge(&edge, contact->GetChildIndexA());
            
            b2WorldManifold world_manifold;
            contact->GetWorldManifold(&world_manifold);
            
            Vector2d edge_normal = toVector<double>(b2Mul(fixture_a->GetBody()->GetTransform().q, b2Vec2(edge.m_vertex2.y - edge.m_vertex1.y, edge.m_vertex1.x - edge.m_vertex2.x))).normalized();
            if (edge_normal.dot(toVector<double>(world_manifold.normal)) > -0.3)
                contact->SetEnabled(false);
        }
        if (fixture_b->GetType() == b2Shape::Type::e_chain)
        {
            b2ChainShape* chain = (b2ChainShape*)fixture_b->GetShape();
            b2EdgeShape edge;
            chain->GetChildEdge(&edge, contact->GetChildIndexB());
            
            b2WorldManifold world_manifold;
            contact->GetWorldManifold(&world_manifold);
            
            Vector2d edge_normal = toVector<double>(b2Mul(fixture_b->GetBody()->GetTransform().q, b2Vec2(edge.m_vertex2.y - edge.m_vertex1.y, edge.m_vertex1.x - edge.m_vertex2.x))).normalized();
            if (edge_normal.dot(toVector<double>(world_manifold.normal)) < 0.3)
                contact->SetEnabled(false);
        }
	}
};

class DestructionListener : public b2DestructionListener
{
	virtual void SayGoodbye(b2Joint* joint)
	{
        Joint2D* my_joint = (Joint2D*)joint->GetUserData();
        if (my_joint)
        {
            my_joint->joint = nullptr;
            delete my_joint;
        }
	}
	
	virtual void SayGoodbye(b2Fixture* fixture)
	{
	}
};

void Shape2D::create(Node* node) const
{
    if (!node->getScene()->collision_world2d)
    {
        node->getScene()->collision_world2d = new b2World(b2Vec2_zero);
        node->getScene()->collision_world2d->SetContactListener(new ContactListener());
        node->getScene()->collision_world2d->SetDestructionListener(new DestructionListener());
    }
    b2World* world = node->getScene()->collision_world2d;

    sp2assert(node->parent == node->getScene()->getRoot(), "2D collision shapes can only be added to top level nodes.");
    sp2assert(node->collision_body3d == nullptr, "When setting a 2D collision shape, the node should not have a 3D collision shape");

    if (!node->collision_body2d)
    {
        b2BodyDef body_def;
        body_def.position = toVector(node->getGlobalPosition2D());
        body_def.angle = node->getGlobalRotation2D() / 180.0 * pi;
        body_def.linearDamping = linear_damping;
        body_def.angularDamping = angular_damping;
        switch(type)
        {
        case Type::Sensor:
            body_def.type = b2_dynamicBody;
            break;
        case Type::Static:
            body_def.type = b2_staticBody;
            break;
        case Type::Kinematic:
            body_def.type = b2_kinematicBody;
            break;
        case Type::Dynamic:
            body_def.type = b2_dynamicBody;
            break;
        }
        body_def.fixedRotation = fixed_rotation;
        body_def.userData = node;
        node->collision_body2d = world->CreateBody(&body_def);
    }
    
    while(node->collision_body2d->GetFixtureList())
        node->collision_body2d->DestroyFixture(node->collision_body2d->GetFixtureList());
    createFixture(node->collision_body2d);
}

void Shape2D::createFixtureOnBody(b2Body* body, b2Shape* shape) const
{
    b2FixtureDef shapeDef;
    shapeDef.shape = shape;
    shapeDef.density = density;
    shapeDef.friction = friction;
    shapeDef.restitution = restitution;
    shapeDef.isSensor = (type == Type::Sensor);
    shapeDef.userData = body->GetUserData();
    shapeDef.filter.categoryBits = filter_category;
    shapeDef.filter.maskBits = filter_mask;
    body->CreateFixture(&shapeDef);
}

};//namespace collision
};//namespace sp
