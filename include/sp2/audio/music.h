#ifndef SP2_AUDIO_MUSIC_H
#define SP2_AUDIO_MUSIC_H

#include <sp2/string.h>

namespace sp {
namespace audio {

class Music
{
public:
    static bool play(const string& resource_name, bool loop=true);
    static bool isPlaying();
    static void stop();

    //Set the music volume in the range of 0 to 100
    static void setVolume(float volume);
    static float getVolume();
};

}//namespace audio
}//namespace sp

#endif//SP2_AUDIO_MUSIC_H
