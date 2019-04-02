#include <sp2/graphics/scene/renderdata.h>

namespace sp {

RenderData::RenderData()
{
    color = Color(1, 1, 1);
    type = Type::None;
    order = 0;
    scale = sp::Vector3f(1.0, 1.0, 1.0);
    shader = nullptr;
    texture = nullptr;
}

bool RenderData::operator<(const RenderData& other) const
{
    if (order < other.order)
        return true;
    if (order > other.order)
        return false;

    if (type < other.type)
        return true;
    if (type > other.type)
        return false;

    if (shader < other.shader)
        return true;
    if (shader > other.shader)
        return false;

    if (texture < other.texture)
        return true;
    if (texture > other.texture)
        return false;

    return false;
}

};//namespace sp
