#include <sp2/audio/musicPlayer.h>
#include <sp2/audio/music.h>
#include <sp2/random.h>
#include <sp2/io/resourceProvider.h>


namespace sp::audio {

MusicPlayer::MusicPlayer(const string& resource_path)
{
    searchResources(resource_path);
}

MusicPlayer::MusicPlayer(std::initializer_list<string> files)
{
    for(const auto& f : files)
        add(f);
}

void MusicPlayer::onUpdate(float delta)
{
    if (Music::isPlaying() || files.empty())
        return;
    Music::play(*sp::randomSelect(files));
}

void MusicPlayer::searchResources(const string& resource_path)
{
    LOG(Info, "Searching", resource_path, "for ogg files");
    for(auto& f : sp::io::ResourceProvider::find(resource_path + "/*.ogg"))
        add(f);
    LOG(Info, files.size(), "music files found.");
}

void MusicPlayer::add(const string& filename)
{
    files.push_back(filename);
}

}//namespace sp::audio
