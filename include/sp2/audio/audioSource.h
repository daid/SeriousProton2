#ifndef SP2_AUDIO_AUDIOSOURCE_H
#define SP2_AUDIO_AUDIOSOURCE_H

#include <sp2/string.h>

struct SDL_AudioStream;

namespace sp {
class Engine;
namespace audio {

/** Base class for objects that want to output sound.
    Generally, this is used by the sound and music modules to output their audio.
    But if you want to create custom generated audio output, this class can be used.
 */
class AudioSource
{
public:
    virtual ~AudioSource();

    void start();
    bool isPlaying();
    void stop();

protected:
    virtual void onMixSamples(float* stream, int sample_count) = 0;
    static void mix(float* stream, const float* source, int sample_count, float volume);
private:
    bool active = false;
    AudioSource* next;
    AudioSource* previous;
    
private:
    static AudioSource* source_list_start;
    static void startAudioSystem();
    static void onAudioCallback(float* stream, int sample_count);
    
    friend class sp::Engine;
    friend void audioCallback(void* userdata, SDL_AudioStream *stream, int additional_amount, int total_amount);
};

}//namespace audio
}//namespace sp

#endif//SP2_AUDIO_AUDIOSOURCE_H
