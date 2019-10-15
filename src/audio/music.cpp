#include <sp2/audio/music.h>
#include <sp2/audio/audioSource.h>
#include <sp2/io/resourceProvider.h>

#include <SDL_audio.h>

#define STB_VORBIS_NO_STDIO
#define STB_VORBIS_NO_PUSHDATA_API
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif//__GNUC__
#include "stb/stb_vorbis.h"
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif//__GNUC__


namespace sp {
namespace audio {

static int mix_volume = SDL_MIX_MAXVOLUME;

class MusicSource : public AudioSource
{
public:
    bool open(io::ResourceStreamPtr stream)
    {
        if (!stream)
            return false;
        stop();
        if (vorbis)
            stb_vorbis_close(vorbis);

        file_data.resize(stream->getSize());
        stream->read(file_data.data(), file_data.size());
        vorbis = stb_vorbis_open_memory(file_data.data(), file_data.size(), nullptr, nullptr);
        if (!vorbis)
        {
            return false;
        }
        //stb_vorbis_info info = stb_vorbis_get_info(vorbis);
        //TODO, if the vorbis file is not 44100Hz we need to convert the output.
        start();
        return true;
    }

protected:
    virtual void onMixSamples(float* stream, int sample_count) override
    {
        float buffer[sample_count];
        int vorbis_samples = stb_vorbis_get_samples_float_interleaved(vorbis, 2, buffer, sample_count);
        mix(stream, buffer, vorbis_samples * 2, mix_volume);
    }

private:
    std::vector<uint8_t> file_data;
    stb_vorbis* vorbis = nullptr;
};

static MusicSource music_source;

bool Music::play(string resource_name)
{
    io::ResourceStreamPtr stream = io::ResourceProvider::get(resource_name);
    if (!stream)
    {
        LOG(Error, "Failed to open", resource_name, "to play as music");
        return false;
    }
    if (!music_source.open(stream))
        return false;
    LOG(Info, "Playing music", resource_name);
    return true;
}

void Music::stop()
{
    music_source.stop();
}

void Music::setVolume(float volume)
{
    mix_volume = volume * SDL_MIX_MAXVOLUME / 100.0f;
}

}//namespace audio
}//namespace sp
