#include <sp2/audio/audioSource.h>
#include <sp2/logging.h>
#include <sp2/assert.h>
#include <string.h>
#include <SDL3/SDL.h>


namespace sp {
namespace audio {

AudioSource* AudioSource::source_list_start;
static SDL_AudioStream* sdl_audio_stream = nullptr;


AudioSource::~AudioSource()
{
    stop();
}

void AudioSource::start()
{
    if (active)
        return;
    SDL_LockAudioStream(sdl_audio_stream);
    active = true;
    next = source_list_start;
    if (next)
        next->previous = this;
    else
        SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(sdl_audio_stream));
    previous = nullptr;
    source_list_start = this;
    SDL_UnlockAudioStream(sdl_audio_stream);
}

bool AudioSource::isPlaying()
{
    return active;
}

void AudioSource::stop()
{
    if (!active)
        return;
    SDL_LockAudioStream(sdl_audio_stream);
    active = false;
    if (source_list_start == this)
    {
        source_list_start = next;
        if (source_list_start)
            source_list_start->previous = nullptr;
        else
            SDL_PauseAudioDevice(SDL_GetAudioStreamDevice(sdl_audio_stream));
    }
    else
    {
        previous->next = next;
    }
    if (next)
        next->previous = previous;
    SDL_UnlockAudioStream(sdl_audio_stream);
}

void audioCallback(void* userdata, SDL_AudioStream *stream, int additional_amount, int total_amount)
{
    if (additional_amount > 0) {
        Uint8* data = SDL_stack_alloc(Uint8, additional_amount);
        if (data) {
            AudioSource::onAudioCallback(reinterpret_cast<float*>(data), additional_amount / sizeof(float));
            SDL_PutAudioStreamData(stream, data, additional_amount);
            SDL_stack_free(data);
        }
    }
}

void AudioSource::startAudioSystem()
{
    SDL_AudioSpec want, have;

    memset(&want, 0, sizeof(want));
    want.freq = 48000;
    want.format = SDL_AUDIO_F32LE;
    want.channels = 2;
    have = want;//TMP

    SDL_InitSubSystem(SDL_INIT_AUDIO);
    sdl_audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &want, audioCallback, nullptr);
    if (sdl_audio_stream == nullptr)
    {
        LOG(Error, SDL_GetError());
        LOG(Warning, "Failed to open audio device, no audio available.");
        return;
    }
    LOG(Info, "Opened audio:", have.freq, int(have.channels));
    sp2assert(have.format == SDL_AUDIO_F32LE, "Needs 32 float audio output.");
    sp2assert(have.freq == 48000, "Needs 48000Hz audio output.");
    sp2assert(have.channels == 2, "Needs 2 channel audio output.");

    //Check if any audio source was started before we opened our audio device.
    if (AudioSource::source_list_start)
        SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(sdl_audio_stream));
}

void AudioSource::onAudioCallback(float* stream, int sample_count)
{
    memset(stream, 0, sample_count * sizeof(float));
    for(AudioSource* source = AudioSource::source_list_start; source; source = source->next)
        source->onMixSamples(stream, sample_count);
}

void AudioSource::mix(float* stream, const float* source, int sample_count, float volume)
{
    SDL_MixAudio(reinterpret_cast<uint8_t*>(stream), reinterpret_cast<const uint8_t*>(source), SDL_AUDIO_F32LE, sample_count * sizeof(float), volume);
}

}//namespace audio
}//namespace sp
