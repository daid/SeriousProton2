#include <sp2/graphics/scene/renderdata.h>

namespace sp {

RenderData::RenderData()
{
    color = Color::White;
    type = Type::None;
    order = 0;
    scale = sp::Vector3f(1.0, 1.0, 1.0);
    shader = nullptr;
}

bool RenderData::operator<(const RenderData& other) const
{
    if (order < other.order)
        return true;
    if (order == other.order)
    {
        if (type < other.type)
            return true;
        if (type == other.type)
        {
            return false;
        }
    }
    return false;
}

};//!namespace sp

