#include <sp2/audio/sound.h>
#include <sp2/audio/audioSource.h>
#include <sp2/io/resourceProvider.h>

#include <SDL3/SDL.h>

#include <unordered_map>
#include <string.h>

namespace sp {
namespace audio {

typedef std::vector<float> AudioBuffer;

static float mix_volume = 1.0f;

class SoundSource : public AudioSource
{
public:
    virtual void onMixSamples(float* stream, int sample_count) override
    {
        sample_count = std::min(sample_count, int(buffer->size() - index));
        mix(stream, &(buffer->data()[index]), sample_count, mix_volume);
        index += sample_count;
        if (index == buffer->size())
        {
            buffer = nullptr;
            stop();
        }
    }

    unsigned int index;
    const AudioBuffer* buffer;
};

static std::unordered_map<string, AudioBuffer*> sound_cache;
static const int slot_count = 32;
static SoundSource slot[slot_count];

static Sint64 SDLCALL sdl_to_stream_size(void* userdata)
{
    sp::io::ResourceStream* stream = static_cast<sp::io::ResourceStream*>(userdata);
    return stream->getSize();
}

static Sint64 SDLCALL sdl_to_stream_seek(void* userdata, Sint64 offset, SDL_IOWhence whence)
{
    sp::io::ResourceStream* stream = static_cast<sp::io::ResourceStream*>(userdata);
    switch(whence)
    {
    case SDL_IO_SEEK_SET: stream->seek(offset); break;
    case SDL_IO_SEEK_CUR: stream->seek(stream->tell() + offset); break;
    case SDL_IO_SEEK_END: stream->seek(stream->getSize() + offset); break;
    }
    return stream->tell();
}

static size_t SDLCALL sdl_to_stream_read(void* userdata, void *ptr, size_t size, SDL_IOStatus* status)
{
    sp::io::ResourceStream* stream = static_cast<sp::io::ResourceStream*>(userdata);
    return stream->read(ptr, size);
}

static size_t SDLCALL sdl_to_stream_write(void* userdata, const void *ptr, size_t size, SDL_IOStatus* status)
{
    *status = SDL_IO_STATUS_READONLY;
    return 0;
}

static bool SDLCALL sdl_to_stream_close(void* userdata)
{
    return true;
}

void Sound::play(const string& resource_name)
{
    AudioBuffer* data = sound_cache[resource_name];
    if (data == nullptr)
    {
        data = new AudioBuffer();
        io::ResourceStreamPtr stream = io::ResourceProvider::get(resource_name);
        if (stream)
        {
            SDL_IOStreamInterface interface;
            SDL_INIT_INTERFACE(&interface);
            interface.size = sdl_to_stream_size;
            interface.seek = sdl_to_stream_seek;
            interface.read = sdl_to_stream_read;
            interface.write = sdl_to_stream_write;
            interface.close = sdl_to_stream_close;

            auto sdl_stream = SDL_OpenIO(&interface, stream.get());

            SDL_AudioSpec spec;
            uint8_t* buffer = nullptr;
            uint32_t buffer_size = 0;
            if (SDL_LoadWAV_IO(sdl_stream, false, &spec, &buffer, &buffer_size))
            {
                uint8_t* target_buffer = nullptr;
                int target_buffer_size = 0;
                SDL_AudioSpec target_spec;
                target_spec.format = SDL_AUDIO_F32LE;
                target_spec.channels = 2;
                target_spec.freq = 48000;
                SDL_ConvertAudioSamples(&spec, buffer, buffer_size, &target_spec, &target_buffer, &target_buffer_size);

                data->resize(target_buffer_size / sizeof(float));
                memcpy(data->data(), target_buffer, target_buffer_size);
                SDL_free(target_buffer);
                SDL_free(buffer);
            }
            SDL_CloseIO(sdl_stream);

            LOG(Info, "Loaded", resource_name, "with", data->size(), "samples", float(data->size()) / 2 / 44100);
        }
        sound_cache[resource_name] = data;
    }

    for(int n=0; n<slot_count; n++)
    {
        if (slot[n].buffer == nullptr)
        {
            slot[n].index = 0;
            slot[n].buffer = data;
            slot[n].start();
            return;
        }
    }
}

void Sound::setVolume(float volume)
{
    mix_volume = volume / 100.0f;
}

float Sound::getVolume()
{
    return float(mix_volume) * 100.0f;
}

}//namespace audio
}//namespace sp
