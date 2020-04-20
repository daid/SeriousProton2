#include <sp2/graphics/color.h>

namespace sp {

HsvColor::HsvColor(const sp::Color& color)
{
    auto max = std::max(std::max(color.r, color.g), color.b);
    auto min = std::min(std::min(color.r, color.g), color.b);

    auto delta = max - min;
    if (delta <= 0.0 || max <= 0.0)
    {
        hue = 0.0;
        saturation = 0.0;
        value = max * 100;
        return;
    }
    hue = 0.0;
    saturation = delta / max * 100.0;
    value = max * 100.0;
    if (color.r == max)
        hue = (color.g - color.b) / delta;
    else if (color.g == max)
        hue = 2.0 + (color.b - color.r) / delta;
    else
        hue = 4.0 + (color.r - color.g) / delta;
    hue *= 60.0;
    if (hue < 0)
        hue += 360.0;
}

}//namespace sp
