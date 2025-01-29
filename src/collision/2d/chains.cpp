#include <sp2/collision/2d/chains.h>
#include <sp2/logging.h>

#include <private/collision/box2dVector.h>
#include <private/collision/box2d.h>


namespace sp {
namespace collision {

Chains2D::Chains2D()
{
    type = Type::Static;
}

void Chains2D::createFixture(b2Body* body) const
{
    for(auto chain : chains)
    {
        b2ChainShape shape;
        std::vector<b2Vec2> verts;
        verts.reserve(chain.size());
        for(unsigned int n=0; n<chain.size(); n++)
            verts.push_back(toVector(chain[n]));
        shape.CreateChain(verts.data(), chain.size());

        createFixtureOnBody(body, &shape);
    }

    for(auto loop : loops)
    {
        b2ChainShape shape;
        std::vector<b2Vec2> verts;
        verts.reserve(loop.size());
        for(unsigned int n=0; n<loop.size(); n++)
            verts.push_back(toVector(loop[n]));
        shape.CreateLoop(verts.data(), loop.size());

        createFixtureOnBody(body, &shape);
    }
}

}//namespace collision
}//namespace sp
