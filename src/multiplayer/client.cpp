#include <sp2/multiplayer/client.h>
#include <sp2/assert.h>

namespace sp {
namespace multiplayer {

Client* Client::instance;
    
Client::Client(string hostname)
{
    sp2assert(!instance, "Only a single multiplayer::Client instance can exists");
    instance = this;
}

Client::~Client()
{
    instance = nullptr;
}
    
void Client::update()
{
}

};//!namespace multiplayer
};//!namespace sp
