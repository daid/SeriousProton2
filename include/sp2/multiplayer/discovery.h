#ifndef SP2_MULTIPLAYER_DISCOVERY_H
#define SP2_MULTIPLAYER_DISCOVERY_H

#include <sp2/updatable.h>
#include <sp2/string.h>
#include <sp2/io/network/udpSocket.h>

namespace sp {
namespace multiplayer {

class Discovery : public Updatable
{
public:
    class ServerInfo
    {
    public:
        enum class Type
        {
            Local,
            Switchboard
        };

        Type type;
        string name;
        int version;

        string local_address;

        string switchboard_hostname;
        int switchboard_port;
        string switchboard_key;
    };

    Discovery(const string& game_name);

    //Start scanning the local network for servers.
    // Note that this initiates the scan, but results will be available later.
    void scanLocalNetwork(int scan_port);
    //Scan a switchboard server. This will directly populate the server list.
    void scanSwitchboard(const string& hostname, int port=80);

    std::vector<ServerInfo> getServers();

    virtual void onUpdate(float delta) override;

private:
    string game_name;
    std::vector<ServerInfo> servers;
};

}//namespace multiplayer
}//namespace sp


#endif//SP2_MULTIPLAYER_DISCOVERY_H
