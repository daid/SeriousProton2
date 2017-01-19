#ifndef SP2_GRAPHICS_COLOR_H
#define SP2_GRAPHICS_COLOR_H

#include <SFML/Graphics/Color.hpp>
#include <sp2/string.h>
#include <sp2/logging.h>
#include <cmath>

namespace sp {

class Color : public sf::Color
{
public:
    Color()
    : sf::Color()
    {
    }

    Color(const sf::Color& color)
    : sf::Color(color)
    {
    }
    
    Color(uint32_t color)
    : sf::Color(color)
    {
    }

    Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
    : sf::Color(red, green, blue, alpha)
    {
    }

    static Color fromString(string s)
    {
        if (s.startswith("#"))
        {
            if (s.length() == 7)
                return Color((s.substr(1).toInt(16) << 8) | 0xFF);
            if (s.length() == 9)
                return Color(s.substr(0, 4).toInt(16) << 16 | s.substr(4).toInt(16));  //toInt(16) fails with 8 bytes, so split the convertion in 2 sections.
        }
        LOG(Error, "Failed to parse color string", s);
        return Color::White;
    }

    //Convert a HSV value into an RGB value.
    // hue: 0-360
    // saturation: 0-100
    // value: 0-100
    static Color fromHSV(double hue, double saturation, double value)
    {
        double c = value / 100.0 * saturation / 100.0;
        double x = c * (1.0 - std::abs(std::fmod((hue / 60.0), 2.0) - 1.0));
        double m = value / 100.0 - c;

        if (hue < 60)
            return Color((c + m) * 255, (x + m) * 255, (m) * 255);
        else if (hue < 120)
            return Color((x + m) * 255, (c + m) * 255, (m) * 255);
        else if (hue < 180)
            return Color((m) * 255, (c + m) * 255, (x + m) * 255);
        else if (hue < 240)
            return Color((m) * 255, (x + m) * 255, (c + m) * 255);
        else if (hue < 300)
            return Color((x + m) * 255, (m) * 255, (c + m) * 255);
        else
            return Color((c + m) * 255, (m) * 255, (x + m) * 255);
        return Color::Black;
    }
private:
};

};//!namespace sp

#endif//SP2_GRAPHICS_OPENGL_H
