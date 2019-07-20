#ifndef SP2_MULTIPLAYER_PACKET_IDS_H
#define SP2_MULTIPLAYER_PACKET_IDS_H

#include <sp2/scene/node.h>

namespace sp {
namespace multiplayer {

class PacketIDs
{
public:
    //Initial packet from the server to the client. Contains magic values and version numbers to make sure we are connected to a proper server.
    // As well as an optional password challange.
    //Client replies with the same packetID, and the response to the password challange if required.
    static constexpr uint8_t request_authentication = 0x01;
    //After the authentication request, the server will send a client id packet to the client.
    //This indicates towards the client which unique number it is.
    static constexpr uint8_t set_client_id = 0x02;
    //When the game speed changes, we need to update this to the clients. As this is an engine global, we have a special handling for this.
    static constexpr uint8_t change_game_speed = 0x03;
    
    //Create a new object, gives the typeid, and all the members to set.
    static constexpr uint8_t create_object = 0x10;
    //Update members of an object
    static constexpr uint8_t update_object = 0x11;
    //Delete a specific object
    static constexpr uint8_t delete_object = 0x12;

    //Scenes should already exist on the client, this packet is to link an multiplayer ID to a scene.
    static constexpr uint8_t setup_scene = 0x20;

    //Packet send from the client to the server to indicate the client wants to call a function on the server.
    static constexpr uint8_t call_on_server = 0x80;
    
    //Alive packet. Empty packet that is being send from the server to indicate that the server is still connected to the client.
    //TCP timeouts will handle disconnecting clients from the server. Clients need to have extra timeout handling.
    static constexpr uint8_t alive = 0xFF;
    
    //Magic value to identify a SP2 server/client.
    static constexpr uint64_t magic_sp2_value = 0x33a7a0651d1394b1;
};

};//namespace multiplayer
};//namespace sp

#endif//SP2_MULTIPLAYER_PACKET_IDS_H
