#ifndef SP2_GRAPHICS_COLOR_H
#define SP2_GRAPHICS_COLOR_H

#include <SFML/Graphics/Color.hpp>
#include <sp2/math/vector.h>
#include <sp2/stringutil/convert.h>
#include <sp2/string.h>
#include <sp2/logging.h>
#include <cmath>

namespace sp {

class Color;

class HsvColor
{
public:
    HsvColor()
    : hue(0), saturation(0), value(0)
    {
    }

    HsvColor(const double hue, const double saturation, const double value)
    : hue(hue), saturation(saturation), value(value)
    {
    }
    
    HsvColor(const sp::Color& color);

    // hue: 0-360
    // saturation: 0-100
    // value: 0-100
    double hue;
    double saturation;
    double value;
};

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
    
    Color(const HsvColor& hsv)
    {
        double c = hsv.value / 100.0 * hsv.saturation / 100.0;
        double x = c * (1.0 - std::abs(std::fmod((hsv.hue / 60.0), 2.0) - 1.0));
        double m = hsv.value / 100.0 - c;

        if (hsv.hue < 60)
        {
            r = (c + m) * 255;
            g = (x + m) * 255;
            b = (m) * 255;
        }
        else if (hsv.hue < 120)
        {
            r = (x + m) * 255;
            g = (c + m) * 255;
            b = (m) * 255;
        }
        else if (hsv.hue < 180)
        {
            r = (m) * 255;
            g = (c + m) * 255;
            b = (x + m) * 255;
        }
        else if (hsv.hue < 240)
        {
            r = (m) * 255;
            g = (x + m) * 255;
            b = (c + m) * 255;
        }
        else if (hsv.hue < 300)
        {
            r = (x + m) * 255;
            g = (m) * 255;
            b = (c + m) * 255;
        }
        else
        {
            r = (c + m) * 255;
            g = (m) * 255;
            b = (x + m) * 255;
        }
    }

    static Color fromString(string s)
    {
        if (s.startswith("#"))
        {
            if (s.length() == 7)
                return Color(stringutil::convert::toInt(s.substr(1), 16) << 8 | 0xFF);
            if (s.length() == 9)
                return Color(stringutil::convert::toInt(s.substr(1, 5), 16) << 16 | stringutil::convert::toInt(s.substr(5), 16));  //toInt(16) fails with 8 bytes, so split the convertion in 2 sections.
        }
        LOG(Error, "Failed to parse color string", s);
        return Color::White;
    }
    
    string toString()
    {
        if (a == 0xFF)
        {
            return "#" + string::hex(toInteger() >> 8, 6);
        }
        else
        {
            return "#" + string::hex(toInteger(), 8);
        }
    }
private:
};

};//!namespace sp

#endif//SP2_GRAPHICS_OPENGL_H
