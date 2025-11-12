#include <sp2/audio/sound.h>
#include <sp2/audio/audioSource.h>
#include <sp2/io/resourceProvider.h>

#include <SDL_audio.h>

#include <unordered_map>
#include <string.h>

#define STB_VORBIS_NO_STDIO
#define STB_VORBIS_NO_PUSHDATA_API
#define STB_VORBIS_HEADER_ONLY
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#pragma GCC diagnostic ignored "-Wshadow-compatible-local"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif//__GNUC__
#include "stb/stb_vorbis.h"
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif//__GNUC__


namespace sp {
namespace audio {

typedef std::vector<float> AudioBuffer;

static int mix_volume = SDL_MIX_MAXVOLUME;

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

static Sint64 SDLCALL sdl_to_stream_size(struct SDL_RWops * context)
{
    sp::io::ResourceStream* stream = static_cast<sp::io::ResourceStream*>(context->hidden.unknown.data1);
    return stream->getSize();
}

static Sint64 SDLCALL sdl_to_stream_seek(struct SDL_RWops * context, Sint64 offset, int whence)
{
    sp::io::ResourceStream* stream = static_cast<sp::io::ResourceStream*>(context->hidden.unknown.data1);
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
    sp::io::ResourceStream* stream = static_cast<sp::io::ResourceStream*>(context->hidden.unknown.data1);
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

static void loadWavFile(io::ResourceStreamPtr stream, AudioBuffer* data)
{
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
        SDL_BuildAudioCVT(&cvt, spec.format, spec.channels, spec.freq, AUDIO_F32, 2, 48000);
        if (cvt.needed)
        {
            cvt.len = buffer_size;
            data->resize(cvt.len * cvt.len_mult / sizeof(float));
            cvt.buf = reinterpret_cast<uint8_t*>(data->data());
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
    }
}

void loadVorbisFile(io::ResourceStreamPtr stream, AudioBuffer* data)
{
    std::vector<uint8_t> file_data;
    file_data.resize(stream->getSize());
    stream->read(file_data.data(), file_data.size());
    stb_vorbis* vorbis = stb_vorbis_open_memory(file_data.data(), file_data.size(), nullptr, nullptr);
    if (!vorbis) return;
    auto info = stb_vorbis_get_info(vorbis);
    size_t sample_count = 0;
    while(true)
    {
        const size_t extra_buffer = 2048;
        data->resize(sample_count + extra_buffer);
        int vorbis_samples = stb_vorbis_get_samples_float_interleaved(vorbis, 2, &(*data)[sample_count], extra_buffer) * 2;
        if (vorbis_samples == 0)
            break;
        sample_count += vorbis_samples;
    }
    if (info.sample_rate != 48000) {
        SDL_AudioCVT cvt;
        SDL_BuildAudioCVT(&cvt, AUDIO_F32, 2, info.sample_rate, AUDIO_F32, 2, 48000);
        cvt.len = sample_count * sizeof(float);
        data->resize(cvt.len * cvt.len_mult / sizeof(float));
        cvt.buf = reinterpret_cast<uint8_t*>(data->data());
        SDL_ConvertAudio(&cvt);
        sample_count = cvt.len_cvt / sizeof(float);
    }
    data->resize(sample_count);
    data->shrink_to_fit();
    stb_vorbis_close(vorbis);
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
            if (resource_name.endswith(".ogg"))
                loadVorbisFile(stream, data);
            else
                loadWavFile(stream, data);
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
    mix_volume = volume * SDL_MIX_MAXVOLUME / 100.0f;
}

float Sound::getVolume()
{
    return float(mix_volume) * 100.0f / SDL_MIX_MAXVOLUME;
}

}//namespace audio
}//namespace sp
