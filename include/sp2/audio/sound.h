#ifndef SP2_AUDIO_SOUND_H
#define SP2_AUDIO_SOUND_H

#include <sp2/string.h>

namespace sp {
namespace audio {

class Sound
{
public:
    static void play(const string& resource_name, float volume=1.0f);
    
    //Set the global volume for all sounds in the range of 0 to 100
    //This does not effect the music volume.
    static void setVolume(float volume);
    static float getVolume();
};

}//namespace audio
}//namespace sp

#endif//SP2_AUDIO_MUSIC_H
