#ifndef SP2_AUDIO_MUSICPLAYER_H
#define SP2_AUDIO_MUSICPLAYER_H

#include <sp2/string.h>
#include <sp2/updatable.h>

namespace sp {
namespace audio {

/* The MusicPlayer is an object that allows playing multiple music files.
 * It plays the files in random order.
 */
class MusicPlayer : public Updatable
{
public:
    MusicPlayer() = default;
    MusicPlayer(const string& resource_path);
    MusicPlayer(std::initializer_list<string> files);

    virtual void onUpdate(float delta) override;

    void searchResources(const string& resource_path);
    void add(const string& filename);
private:
    std::vector<string> files;
};

}//namespace audio
}//namespace sp

#endif//SP2_AUDIO_MUSICPLAYER_H
