#include <sp2/graphics/scene/renderdata.h>

namespace sp {

RenderData::RenderData()
{
    color = sf::Color::White;
    type = Type::None;
    order = 0;
}

bool RenderData::operator<(const RenderData& other) const
{
    if (order < other.order)
        return true;
    if (type < other.type)
        return true;
    return false;
}

};//!namespace sp

