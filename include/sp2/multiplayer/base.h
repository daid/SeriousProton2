#ifndef SP2_MULTIPLAYER_BASE_H
#define SP2_MULTIPLAYER_BASE_H

#include <sp2/pointer.h>
#include <unordered_map>

namespace sp {
class Node;
namespace multiplayer {

/**
 * Base class for both client and server of the multiplayer component.
 *  Manages common functionality and data between client and server implementation.
 */
class Base
{
public:
    P<Node> getNode(uint64_t id);

    float getNetworkDelay() { return network_delay; }

    //Get our client ID assigned by the server. If this is zero, we are the server.
    virtual uint32_t getClientId() = 0;

    bool isServer();
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

}//namespace multiplayer
}//namespace sp

#endif//SP2_MULTIPLAYER_OBJECT_REGISTRY_H
