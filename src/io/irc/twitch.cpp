#include <sp2/io/irc/twitch.h>

namespace sp {
namespace io {

Twitch::Twitch(const string& nick, const string& password)
: IrcClient("irc.chat.twitch.tv", nick, nick, password)
{

}

void Twitch::onConnected(const string& welcome_message)
{
    sendRaw("CAP REQ :twitch.tv/commands");
}

void Twitch::onWhisper(const string& user, const string& message)
{
}

void Twitch::processLine(string prefix, string command, string params, string trailing)
{
    if (command == "WHISPER")
    {
        if (prefix.find("!") != -1)
            prefix = prefix.substr(0, prefix.find("!"));
        onWhisper(prefix, trailing);
    }
    else
    {
        IrcClient::processLine(prefix, command, params, trailing);
    }
}

}//namespace io
}//namespace sp
