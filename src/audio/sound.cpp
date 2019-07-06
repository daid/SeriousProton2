#include <sp2/audio/sound.h>
#include <sp2/audio/audioSource.h>
#include <sp2/io/resourceProvider.h>

#include <SDL_audio.h>

#include <unordered_map>
#include <string.h>

namespace sp {
namespace audio {

typedef std::vector<float> AudioBuffer;

static int mix_volume = SDL_MIX_MAXVOLUME;

class SoundSource : public AudioSource
{
public:
    virtual void onMixSamples(float* stream, int sample_count)
    {
        sample_count = std::min(sample_count, int(buffer->size() - index));
        mix(stream, &(*buffer)[index], sample_count, mix_volume);
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

static Sint64 SDLCALL sdl_to_stream_size(struct SDL_RWops * context)
{
    sp::io::ResourceStream* stream = (sp::io::ResourceStream*)context->hidden.unknown.data1;
    return stream->getSize();
}

static Sint64 SDLCALL sdl_to_stream_seek(struct SDL_RWops * context, Sint64 offset, int whence)
{
    sp::io::ResourceStream* stream = (sp::io::ResourceStream*)context->hidden.unknown.data1;
    switch(whence)
    {
    case RW_SEEK_SET: stream->seek(offset); break;
    case RW_SEEK_CUR: stream->seek(stream->tell() + offset); break;
    case RW_SEEK_END: stream->seek(stream->getSize() + offset); break;
    }
    return stream->tell();
}

static size_t SDLCALL sdl_to_stream_read(struct SDL_RWops * context, void *ptr, size_t size, size_t maxnum)
{
    sp::io::ResourceStream* stream = (sp::io::ResourceStream*)context->hidden.unknown.data1;
    return stream->read(ptr, size * maxnum) / size;
}

static size_t SDLCALL sdl_to_stream_write(struct SDL_RWops * context, const void *ptr, size_t size, size_t num)
{
    return 0;
}

static int SDLCALL sdl_to_stream_close(struct SDL_RWops * context)
{
    return 0;
}

void Sound::play(string resource_name)
{
    AudioBuffer* data = sound_cache[resource_name];
    if (data == nullptr)
    {
        data = new AudioBuffer();
        io::ResourceStreamPtr stream = io::ResourceProvider::get(resource_name);
        SDL_RWops ops = {
            .size = sdl_to_stream_size,
            .seek = sdl_to_stream_seek,
            .read = sdl_to_stream_read,
            .write = sdl_to_stream_write,
            .close = sdl_to_stream_close,
            .type = SDL_RWOPS_UNKNOWN,
            .hidden = {.unknown = { .data1 = stream.get() } },
        };
        SDL_AudioSpec spec;
        uint8_t* buffer = nullptr;
        uint32_t buffer_size = 0;
        if (SDL_LoadWAV_RW(&ops, false, &spec, &buffer, &buffer_size))
        {
            SDL_AudioCVT cvt;
            SDL_BuildAudioCVT(&cvt, spec.format, spec.channels, spec.freq, AUDIO_F32, 2, 44100);
            if (cvt.needed)
            {
                cvt.len = buffer_size;
                data->resize(cvt.len * cvt.len_mult / sizeof(float));
                cvt.buf = (uint8_t*)data->data();
                memcpy(cvt.buf, buffer, buffer_size);
                SDL_ConvertAudio(&cvt);
                data->resize(cvt.len_cvt / sizeof(float));
                data->shrink_to_fit();
            }
            else
            {
                data->resize(buffer_size / sizeof(float));
                memcpy(data->data(), buffer, buffer_size);
            }
            SDL_FreeWAV(buffer);

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
    mix_volume = volume * SDL_MIX_MAXVOLUME / 100.0;
}

};//namespace audio
};//namespace sp
