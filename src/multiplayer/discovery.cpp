#include <sp2/multiplayer/discovery.h>
#include <sp2/io/http/request.h>
#include <sp2/assert.h>

#include <json11/json11.hpp>


namespace sp {
namespace multiplayer {

Discovery::Discovery(const string& game_name)
: game_name(game_name)
{
}

void Discovery::scanLocalNetwork(int scan_port)
{
    sp2assert(false, "Local network scanning not implemented yet");
}

void Discovery::scanSwitchboard(const string& hostname, int port)
{
    sp::io::http::Request request("daid.eu", 32032);
    auto response = request.get("/game/list/" + game_name);
    if (response.status != 200)
    {
        LOG(Error, "Switchboard response was not a http 200 but:", response.status);
        return;
    }

    std::string err;
    auto json = json11::Json::parse(response.body, err);
    if (!err.empty())
    {
        LOG(Error, "Unknown response from switchboard server:", response.body);
        return;
    }

    servers.erase(std::remove_if(servers.begin(), servers.end(), [&hostname, port](const ServerInfo& i)
    {
        return i.type == ServerInfo::Type::Switchboard
            && i.switchboard_hostname == hostname
            && i.switchboard_port == port;
    }), servers.end());
    for(auto& json_entry : json.array_items())
    {
        servers.emplace_back();
        ServerInfo& info = servers.back();
        info.type = ServerInfo::Type::Switchboard;
        info.name = json_entry["name"].string_value();
        info.switchboard_hostname = hostname;
        info.switchboard_port = port;
        info.switchboard_key = json_entry["key"].string_value();
    }
}

std::vector<Discovery::ServerInfo> Discovery::getServers()
{
    return servers;
}

void Discovery::onUpdate(float delta)
{
}

}//namespace multiplayer
}//namespace sp
