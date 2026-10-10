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

void Chains2D::createShape(b2BodyId body) const
{
    auto shapedef = getShapeDef(body);
    for(auto chain : chains)
    {
        b2ChainDef shape = b2DefaultChainDef();
        std::vector<b2Vec2> verts;
        verts.reserve(chain.size());
        for(unsigned int n=0; n<chain.size(); n++)
            verts.push_back(toVector(chain[chain.size()-1-n]));
        shape.points = verts.data();
        shape.count = verts.size();
        shape.materials = &shapedef.material;
        shape.filter = shapedef.filter;
        shape.userData = shapedef.userData;

        b2CreateChain(body, &shape);
    }

    for(auto loop : loops)
    {
        b2ChainDef shape = b2DefaultChainDef();
        std::vector<b2Vec2> verts;
        verts.reserve(loop.size());
        for(unsigned int n=0; n<loop.size(); n++)
            verts.push_back(toVector(loop[loop.size()-1-n]));
        shape.isLoop = true;
        shape.points = verts.data();
        shape.count = verts.size();
        shape.materials = &shapedef.material;
        shape.filter = shapedef.filter;
        shape.userData = shapedef.userData;

        b2CreateChain(body, &shape);
    }
}

}//namespace collision
}//namespace sp
