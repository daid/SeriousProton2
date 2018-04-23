#include <sp2/audio/sound.h>
#include <sp2/io/resourceProvider.h>

#include <SFML/Audio.hpp>

#include <unordered_map>

namespace sp {
namespace audio {

static std::unordered_map<string, sf::SoundBuffer*> sound_cache;
static std::vector<sf::Sound> sound_list;
static constexpr int sound_list_size = 128;

void Sound::play(string resource_name)
{
    sf::SoundBuffer* data = sound_cache[resource_name];
    if (data == nullptr)
    {
        data = new sf::SoundBuffer();

        io::ResourceStreamPtr stream = io::ResourceProvider::get(resource_name);
        if (!stream) stream = io::ResourceProvider::get(resource_name);
        if (stream && data->loadFromStream(*stream))
        {
            LOG(Info, "Loaded:", resource_name, "of", data->getDuration().asSeconds(), "seconds");
        }else{
            LOG(Warning, "Failed to load sound:", resource_name);
        }
        sound_cache[resource_name] = data;
    }
    if (sound_list.size() == 0)
        sound_list.resize(sound_list_size);
    
    for(sf::Sound& sound : sound_list)
    {
        if (sound.getStatus() == sf::Sound::Stopped)
        {
            sound.setBuffer(*data);
            sound.play();
            return;
        }
    }
}

void Sound::setVolume(float volume)
{
    if (sound_list.size() == 0)
        sound_list.resize(sound_list_size);

    for(sf::Sound& sound : sound_list)
    {
        sound.setVolume(volume);
    }
}

};//namespace audio
};//namespace sp
