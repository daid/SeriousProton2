#include <sp2/stringutil/convert.h>
#include <sp2/logging.h>

namespace sp {
namespace stringutil {
namespace convert {

std::vector<int> toIntArray(const string& s)
{
    std::vector<string> parts = s.split(",");
    std::vector<int> result;
    for(const string& part : parts)
    {
        result.push_back(toInt(part.strip()));
    }
    return result;
}

std::vector<float> toFloatArray(const string& s)
{
    std::vector<string> parts = s.split(",");
    std::vector<float> result;
    for(const string& part : parts)
    {
        result.push_back(toFloat(part.strip()));
    }
    return result;
}

Vector2d toVector2d(const string& s)
{
    double f = toFloat(s);
    if (s.find(",") > -1)
    {
        return Vector2d(f, toFloat(s.substr(s. find(",") + 1).strip()));
    }
    return Vector2d(f, f);
}

Vector2f toVector2f(const string& s)
{
    float f = toFloat(s);
    if (s.find(",") > -1)
    {
        return Vector2f(f, toFloat(s.substr(s. find(",") + 1).strip()));
    }
    return Vector2f(f, f);
}

Vector2i toVector2i(const string& s)
{
    int i = toInt(s);
    if (s.find(",") > -1)
    {
        return Vector2i(i, toInt(s.substr(s. find(",") + 1).strip()));
    }
    return Vector2i(i, i);
}

Alignment toAlignment(const string& s)
{
    string value = s.lower();
    if (value == "topleft" || value == "lefttop") return Alignment::TopLeft;
    if (value == "top" || value == "topcenter" || value == "centertop") return Alignment::Top;
    if (value == "topright" || value == "righttop") return Alignment::TopRight;
    if (value == "left" || value == "leftcenter" || value == "centerleft") return Alignment::Left;
    if (value == "center") return Alignment::Center;
    if (value == "right" || value == "rightcenter" || value == "centerright") return Alignment::Right;
    if (value == "bottomleft" || value == "leftbottom") return Alignment::BottomLeft;
    if (value == "bottom" || value == "bottomcenter" || value == "centerbottom") return Alignment::Bottom;
    if (value == "bottomright" || value == "rightbottom") return Alignment::BottomRight;
    LOG(Warning, "Unknown alignment:", value);
    return Alignment::Center;
}

}//namespace convert
}//namespace stringutil
}//namespace sp
