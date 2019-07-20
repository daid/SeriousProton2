#include <sp2/audio/audioSource.h>
#include <sp2/logging.h>
#include <sp2/assert.h>
#include <string.h>
#include <SDL_audio.h>


namespace sp {
namespace audio {

AudioSource* AudioSource::source_list_start;
static SDL_AudioDeviceID sdl_audio_device = 0;


AudioSource::~AudioSource()
{
    stop();
}

void AudioSource::start()
{
    if (active)
        return;
    SDL_LockAudioDevice(sdl_audio_device);
    active = true;
    next = source_list_start;
    if (next)
        next->previous = this;
    else
        SDL_PauseAudioDevice(sdl_audio_device, 0);
    previous = nullptr;
    source_list_start = this;
    SDL_UnlockAudioDevice(sdl_audio_device);
}

void AudioSource::stop()
{
    if (!active)
        return;
    SDL_LockAudioDevice(sdl_audio_device);
    active = false;
    if (source_list_start == this)
    {
        source_list_start = next;
        if (source_list_start)
            source_list_start->previous = nullptr;
        else
            SDL_PauseAudioDevice(sdl_audio_device, 1);
    }
    else
    {
        previous->next = next;
    }
    if (next)
        next->previous = previous;
    SDL_UnlockAudioDevice(sdl_audio_device);
}

void audioCallback(void* userdata, uint8_t* stream, int length)
{
    AudioSource::onAudioCallback((float*)stream, length / sizeof(float));
}

void AudioSource::startAudioSystem()
{
    SDL_AudioSpec want, have;

    memset(&want, 0, sizeof(want));
    want.freq = 44100;
    want.format = AUDIO_F32;
    want.channels = 2;
    want.samples = 1024;
    want.callback = audioCallback;

    sdl_audio_device = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
    if (sdl_audio_device == 0)
    {
        LOG(Warning, "Failed to open audio device, no audio available.");
        return;
    }
    LOG(Info, "Opened audio:", have.freq, int(have.channels));
    sp2assert(have.format == AUDIO_F32, "Needs 32 float audio output.");
    sp2assert(have.freq == 44100, "Needs 44100Hz audio output.");
    sp2assert(have.channels == 2, "Needs 2 channel audio output.");
}

void AudioSource::onAudioCallback(float* stream, int sample_count)
{
    memset(stream, 0, sample_count * sizeof(float));
    for(AudioSource* source = AudioSource::source_list_start; source; source = source->next)
        source->onMixSamples(stream, sample_count);
}

void AudioSource::mix(float* stream, const float* source, int sample_count, int volume)
{
    SDL_MixAudioFormat((uint8_t*)stream, (const uint8_t*)source, AUDIO_F32, sample_count * sizeof(float), volume);
}

};//namespace audio
};//namespace sp
