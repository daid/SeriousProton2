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

Vector3d toVector3d(const string& s)
{
    double x = toFloat(s);
    double y = x;
    double z = x;
    if (s.find(",") > -1)
    {
        y = toFloat(s.substr(s. find(",") + 1).strip());
        z = y;
        if (s.find(",", s.find(",") + 1) > -1)
        {
            z = toFloat(s.substr(s. find(",", s.find(",") + 1) + 1).strip());
        }
    }
    return Vector3d(x, y, z);
}

Vector3f toVector3f(const string& s)
{
    float x = toFloat(s);
    float y = x;
    float z = x;
    if (s.find(",") > -1)
    {
        y = toFloat(s.substr(s. find(",") + 1).strip());
        z = y;
        if (s.find(",", s.find(",") + 1) > -1)
        {
            z = toFloat(s.substr(s. find(",", s.find(",") + 1) + 1).strip());
        }
    }
    return Vector3f(x, y, z);
}

Vector3i toVector3i(const string& s)
{
    int x = toInt(s);
    int y = x;
    int z = x;
    if (s.find(",") > -1)
    {
        y = toInt(s.substr(s. find(",") + 1).strip());
        z = y;
        if (s.find(",", s.find(",") + 1) > -1)
        {
            z = toInt(s.substr(s. find(",", s.find(",") + 1) + 1).strip());
        }
    }
    return Vector3i(x, y, z);
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

Color toColor(const string& s)
{
    if (s.startswith("#"))
    {
        if (s.length() == 7)
            return Color(float(stringutil::convert::toInt(s.substr(1, 3), 16)) / 255.0, float(stringutil::convert::toInt(s.substr(3, 5), 16)) / 255.0, float(stringutil::convert::toInt(s.substr(5, 7), 16)) / 255.0);
        if (s.length() == 9)
            return Color(float(stringutil::convert::toInt(s.substr(1, 3), 16)) / 255.0, float(stringutil::convert::toInt(s.substr(3, 5), 16)) / 255.0, float(stringutil::convert::toInt(s.substr(5, 7), 16)) / 255.0, float(stringutil::convert::toInt(s.substr(7, 9), 16)) / 255.0);
    }
    LOG(Error, "Failed to parse color string", s);
    return Color(1, 1, 1);
}

}//namespace convert
}//namespace stringutil
}//namespace sp
