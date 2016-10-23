#include <sp2/graphics/scene/renderdata.h>

namespace sp {

RenderData::RenderData()
{
    type = Type::None;
}

bool RenderData::operator<(const RenderData& other) const
{
    if (type < other.type)
        return true;
    return false;
}

};//!namespace sp

