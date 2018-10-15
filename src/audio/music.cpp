#include <sp2/audio/music.h>
#include <sp2/io/resourceProvider.h>

#include <SFML/Audio.hpp>

#define STB_VORBIS_NO_STDIO
#define STB_VORBIS_NO_PUSHDATA_API
#include "stb/stb_vorbis.h"


namespace sp {
namespace audio {

class VorbisSoundStream : public sf::SoundStream
{
public:
    bool open(io::ResourceStreamPtr stream)
    {
        if (!stream)
            return false;
        if (vorbis)
            stb_vorbis_close(vorbis);
        
        file_data.resize(stream->getSize());
        stream->read(file_data.data(), file_data.size());
        vorbis = stb_vorbis_open_memory(file_data.data(), file_data.size(), nullptr, nullptr);
        if (!vorbis)
            return false;
        stb_vorbis_info info = stb_vorbis_get_info(vorbis);
        sample_buffer.resize(info.sample_rate);
        initialize(2, info.sample_rate);
        return true;
    }
protected:
    virtual bool onGetData(sf::SoundStream::Chunk& data)
    {
        data.samples = &sample_buffer[0];
        data.sampleCount = stb_vorbis_get_samples_short_interleaved(vorbis, 2, &sample_buffer[0], sample_buffer.size()) * 2;
        return true;
    }
    
    virtual void onSeek(sf::Time timeOffset)
    {
        if (timeOffset.asMicroseconds() == 0)
            stb_vorbis_seek_start(vorbis);
    }

private:
    std::vector<uint8_t> file_data;
    std::vector<int16_t> sample_buffer;
    stb_vorbis* vorbis = nullptr;
};

static VorbisSoundStream music;

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
    if (!music.open(stream))
        return false;
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
