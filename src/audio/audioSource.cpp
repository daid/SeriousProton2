#include <sp2/audio/audioSource.h>
#include <sp2/logging.h>
#include <sp2/assert.h>
#include <string.h>
#include <SDL_audio.h>


namespace sp {
namespace audio {

AudioSource* AudioSource::source_list_start;

AudioSource::~AudioSource()
{
    stop();
}

void AudioSource::start()
{
    if (active)
        return;
    SDL_LockAudio();
    active = true;
    next = source_list_start;
    if (next)
        next->previous = this;
    previous = nullptr;
    source_list_start = this;
    SDL_UnlockAudio();
}

void AudioSource::stop()
{
    if (!active)
        return;
    SDL_LockAudio();
    active = false;
    if (source_list_start == this)
    {
        source_list_start = next;
        if (source_list_start)
            source_list_start->previous = nullptr;
    }
    else
    {
        previous->next = next;
    }
    if (next)
        next->previous = previous;
    SDL_UnlockAudio();
}

void audioCallback(void* userdata, uint8_t* stream, int length)
{
    AudioSource::onAudioCallback((float*)stream, length / sizeof(float));
}

void AudioSource::startAudioSystem()
{
#ifdef ANDROID
#warning TODO, Android has no audio support at the moment, as it seems to need different format support
#else
    SDL_AudioSpec want, have;

    memset(&want, 0, sizeof(want));
    want.freq = 44100;
    want.format = AUDIO_F32;
    want.channels = 2;
    want.samples = 1024;
    want.callback = audioCallback;

    int result = SDL_OpenAudio(&want, &have);
    sp2assert(result == 0, "SDL_OpenAudio failed");
    LOG(Info, "Opened audio:", have.format, have.freq, have.channels);
    SDL_PauseAudio(0);
    sp2assert(have.format == AUDIO_F32, "Needs 32 float audio output.");
    sp2assert(have.freq == 44100, "Needs 44100Hz audio output.");
    sp2assert(have.channels == 2, "Needs 2 channel audio output.");
#endif
}

void AudioSource::onAudioCallback(float* stream, int sample_count)
{
    memset(stream, 0, sample_count * sizeof(float));
    for(AudioSource* source = AudioSource::source_list_start; source; source = source->next)
        source->onMixSamples(stream, sample_count);
}

};//namespace audio
};//namespace sp
