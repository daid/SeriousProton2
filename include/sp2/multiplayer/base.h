#ifndef SP2_MULTIPLAYER_NODE_REGISTRY_H
#define SP2_MULTIPLAYER_NODE_REGISTRY_H

#include <sp2/pointer.h>
#include <unordered_map>

namespace sp {
class Node;
namespace multiplayer {

class NodeRegistry
{
public:
    P<Node> getNode(uint64_t id);

    float getNetworkDelay() { return network_delay; }
protected:
    void addNode(P<Node> node);    
    void cleanDeletedNodes();
    
    std::unordered_map<uint64_t, P<Node>>::iterator nodeBegin();
    std::unordered_map<uint64_t, P<Node>>::iterator nodeEnd();
    
    float network_delay = 0.0;
private:

    //Called when a node that was added is deleted now and removed from the registry.
    virtual void onDeleted(uint64_t id);
    
    std::unordered_map<uint64_t, P<Node>> node_by_id;
};

};//namespace multiplayer
};//namespace sp

#endif//SP2_MULTIPLAYER_OBJECT_REGISTRY_H
