#include <sp2/graphics/texture.h>
#include <sp2/logging.h>


namespace sp {

const sf::Texture* Texture::get()
{
    std::lock_guard<std::mutex> lock(mutex);
    if (image)
    {
        LOG(Info, "Loaded image", name, image->getSize().x, "x", image->getSize().y);
        if (!texture.loadFromImage(*image))
        {
            LOG(Warning, "loadFromImage failed for", name);
        }
        image = nullptr;
        revision++;
    }
    return &texture;
}

};//!namespace sp
