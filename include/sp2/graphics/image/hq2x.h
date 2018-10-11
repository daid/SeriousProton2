#ifndef SP2_GRAPHICS_IMAGE_HQ2X_H
#define SP2_GRAPHICS_IMAGE_HQ2X_H

#include <sp2/graphics/image.h>
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

void hq2x(sp::Image& image, HQ2xConfig flags);
void hq2xTiles(sp::Image& image, sp::Vector2i tile_size, HQ2xConfig flags);

};//namespace image
};//namespace sp

#endif//SP2_GRAPHICS_IMAGE_HQ2X_H
