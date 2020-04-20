#ifndef SP2_IO_IRC_TWITCH_H
#define SP2_IO_IRC_TWITCH_H

#include <sp2/io/irc/irc.h>


namespace sp {
namespace io {

class Twitch : public Irc
{
public:
    Twitch(const string& nick, const string& password);

    virtual void onConnected(const string& welcome_message) override;

    virtual void onWhisper(const string& user, const string& message);

protected:
    virtual void processLine(string prefix, string command, string params, string trailing) override;
};

}//namespace io
}//namespace sp

#endif//SP2_IO_IRC_TWITCH_H
