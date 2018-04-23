#include <sp2/audio/music.h>
#include <sp2/io/resourceProvider.h>

#include <SFML/Audio.hpp>

namespace sp {
namespace audio {

static io::ResourceStreamPtr music_stream;
static sf::Music music;

bool Music::play(string resource_name)
{
    io::ResourceStreamPtr stream = io::ResourceProvider::get(resource_name);
    if (!stream)
    {
        LOG(Error, "Failed to open", resource_name, "to play as music");
        return false;
    }
    LOG(Info, "Playing music", resource_name);
    music.stop();
    if (!music.openFromStream(*stream))
        return false;
    music_stream = stream;
    music.setLoop(true);
    music.play();
    return true;
}

void Music::stop()
{
    music.stop();
}
    
void Music::setVolume(float volume)
{
    music.setVolume(volume);
}

};//namespace audio
};//namespace sp
