#include <sp2/multiplayer/nodeRegistry.h>
#include <sp2/scene/node.h>
#include <sp2/assert.h>

namespace sp {
namespace multiplayer {

P<Node> NodeRegistry::getNode(uint64_t id)
{
    auto it = node_by_id.find(id);
    if (it == node_by_id.end())
        return nullptr;
    return it->second;
}

void NodeRegistry::addNode(P<Node> node)
{
    sp2assert(node->multiplayer.getId() > 0, "Nodes need to be given an ID before added to a multiplayer node registry.");
    node_by_id[node->multiplayer.getId()] = node;
}

std::unordered_map<uint64_t, P<Node>>::iterator NodeRegistry::nodeBegin()
{
    cleanDeletedNodes();
    return node_by_id.begin();
}

std::unordered_map<uint64_t, P<Node>>::iterator NodeRegistry::nodeEnd()
{
    return node_by_id.end();
}

void NodeRegistry::cleanDeletedNodes()
{
    for(auto it = node_by_id.begin(); it != node_by_id.end(); )
    {
        if (it->second)
        {
            ++it;
        }
        else
        {
            onDeleted(it->first);
            it = node_by_id.erase(it);
        }
    }
}

void NodeRegistry::onDeleted(uint64_t id)
{
}

};//!namespace multiplayer
};//!namespace sp
