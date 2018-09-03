#ifndef SP2_GRAPHICS_IMAGE_HQ2X_H
#define SP2_GRAPHICS_IMAGE_HQ2X_H

#include <SFML/Graphics/Image.hpp>
#include <sp2/math/vector2.h>

namespace sp {
namespace image {

struct HQ2xConfig
{
    enum class OutOfBounds
    {
        Clamp,
        Wrap,
        Transparent,
    } out_of_bounds = OutOfBounds::Wrap;
    int scale = 2;
};

void hq2x(sf::Image& image, HQ2xConfig flags);
void hq2xTiles(sf::Image& image, sp::Vector2i tile_size, HQ2xConfig flags);

};//namespace image
};//namespace sp

#endif//SP2_GRAPHICS_IMAGE_HQ2X_H
