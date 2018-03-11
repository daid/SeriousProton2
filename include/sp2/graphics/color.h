#ifndef SP2_GRAPHICS_COLOR_H
#define SP2_GRAPHICS_COLOR_H

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

class Color
{
public:
    //rgba: 0.0-1.0
    float r, g, b, a;

    Color()
    : r(0), g(0), b(0), a(1.0)
    {
    }

    explicit Color(uint32_t color)
    : r(((color >> 24) & 0xFF) / 255.0f), g(((color >> 16) & 0xFF) / 255.0f), b(((color >> 8) & 0xFF) / 255.0f), a(((color >> 0) & 0xFF) / 255.0f)
    {
        
    }

    Color(float red, float green, float blue, float alpha = 1.0f)
    : r(red), g(green), b(blue), a(alpha)
    {
    }
    
    Color(const HsvColor& hsv)
    {
        double c = hsv.value / 100.0 * hsv.saturation / 100.0;
        double x = c * (1.0 - std::abs(std::fmod((hsv.hue / 60.0), 2.0) - 1.0));
        double m = hsv.value / 100.0 - c;

        if (hsv.hue < 60)
        {
            r = c + m;
            g = x + m;
            b = m;
        }
        else if (hsv.hue < 120)
        {
            r = x + m;
            g = c + m;
            b = m;
        }
        else if (hsv.hue < 180)
        {
            r = m;
            g = c + m;
            b = x + m;
        }
        else if (hsv.hue < 240)
        {
            r = m;
            g = x + m;
            b = c + m;
        }
        else if (hsv.hue < 300)
        {
            r = x + m;
            g = m;
            b = c + m;
        }
        else
        {
            r = c + m;
            g = m;
            b = x + m;
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
        return Color(1, 1, 1);
    }
    
    string toString()
    {
        if (a == 1.0f)
        {
            return "#" + string::hex(toInt() >> 8, 6);
        }
        else
        {
            return "#" + string::hex(toInt(), 8);
        }
    }
    
    uint32_t toInt()
    {
        return (int(r * 255) << 24) | (int(g * 255) << 16) | (int(b * 255) << 8) | int(a * 255);
    }
private:
};

};//!namespace sp

#endif//SP2_GRAPHICS_OPENGL_H
