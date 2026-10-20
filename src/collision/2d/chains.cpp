#include <sp2/collision/2d/chains.h>
#include <sp2/logging.h>

#include <private/collision/box2dVector.h>
#include <private/collision/box2d.h>

#define EPSILON 0.00001


namespace sp {
namespace collision {

Chains2D::Chains2D()
{
    type = Type::Static;
}

void Chains2D::addLine(sp::Vector2f start, sp::Vector2f end)
{
    for(size_t idx=0; idx<chains.size(); idx++)
    {
        auto& chain = chains[idx];
        auto delta = chain.back() - start;
        if (delta.dot(delta) < EPSILON) {
            addToChainEnd(idx, end);
            return;
        }
        delta = chain.front() - end;
        if (delta.dot(delta) < EPSILON) {
            addToChainStart(idx, start);
            return;
        }
    }
    chains.push_back({start, end});
}

void Chains2D::addToChainEnd(size_t idx, sp::Vector2f point)
{
    auto delta = chains[idx].front() - point;
    if (chains[idx].size() > 2 && delta.dot(delta) < EPSILON) { // turn into loop
        loops.push_back(std::move(chains[idx]));
        if (idx < chains.size() - 1) {
            chains[idx] = std::move(chains.back());
        }
        chains.pop_back();
    } else {
        // Check if we merge with another chain.
        for(size_t idx2=0; idx2<chains.size(); idx2++) {
            if (idx == idx2) continue;
            delta = chains[idx2].front() - point;
            if (delta.dot(delta) < EPSILON) {
                chains[idx].insert(chains[idx].end(), chains[idx2].begin(), chains[idx2].end());
                if (idx2 < chains.size() - 1) {
                    chains[idx2] = std::move(chains.back());
                    if (idx == chains.size() - 1)
                        idx = idx2;
                }
                chains.pop_back();
                // Check if we turn into a loop after connecting chains
                delta = chains[idx].front() - chains[idx].back();
                if (chains[idx].size() > 3 && delta.dot(delta) < EPSILON) {
                    chains[idx].pop_back();
                    loops.push_back(std::move(chains[idx]));
                    if (idx < chains.size() - 1) {
                        chains[idx] = std::move(chains.back());
                    }
                    chains.pop_back();
                }
                return;
            }
        }
        chains[idx].push_back(point);
    }
}

void Chains2D::addToChainStart(size_t idx, sp::Vector2f point)
{
    auto delta = chains[idx].back() - point;
    if (chains[idx].size() > 2 && delta.dot(delta) < EPSILON) { // turn into loop
        loops.push_back(std::move(chains[idx]));
        if (idx < chains.size() - 1) {
            chains[idx] = std::move(chains.back());
        }
        chains.pop_back();
    } else {
        // Check if we merge with another chain.
        for(size_t idx2=0; idx2<chains.size(); idx2++) {
            if (idx == idx2) continue;
            delta = chains[idx2].back() - point;
            if (delta.dot(delta) < EPSILON) {
                chains[idx2].insert(chains[idx2].end(), chains[idx].begin(), chains[idx].end());
                if (idx < chains.size() - 1) {
                    chains[idx] = std::move(chains.back());
                    if (idx2 == chains.size() - 1)
                        idx2 = idx;
                }
                chains.pop_back();
                // Check if we turn into a loop after connecting chains
                delta = chains[idx2].front() - chains[idx2].back();
                if (chains[idx2].size() > 3 && delta.dot(delta) < EPSILON) {
                    chains[idx2].pop_back();
                    loops.push_back(std::move(chains[idx2]));
                    if (idx < chains.size() - 1) {
                        chains[idx2] = std::move(chains.back());
                    }
                    chains.pop_back();
                }
                return;
            }
        }
        chains[idx].insert(chains[idx].begin(), point);
    }
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
