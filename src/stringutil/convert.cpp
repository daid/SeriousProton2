#include <sp2/stringutil/convert.h>

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

sp::Vector2d toVector2d(const string& s)
{
    double f = toFloat(s);
    if (s.find(",") > -1)
    {
        return sp::Vector2d(f, toFloat(s.substr(s. find(",") + 1).strip()));
    }
    return sp::Vector2d(f, f);
}

sf::Vector2f toVector2f(const string& s)
{
    float f = toFloat(s);
    if (s.find(",") > -1)
    {
        return sf::Vector2f(f, toFloat(s.substr(s. find(",") + 1).strip()));
    }
    return sf::Vector2f(f, f);
}

sf::Vector2i toVector2i(const string& s)
{
    int i = toInt(s);
    if (s.find(",") > -1)
    {
        return sf::Vector2i(i, toInt(s.substr(s. find(",") + 1).strip()));
    }
    return sf::Vector2i(i, i);
}

};//!namespace convert
};//!namespace stringutil
};//!namespace sp
