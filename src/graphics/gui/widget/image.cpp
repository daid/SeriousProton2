#include <sp2/graphics/gui/widget/image.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/engine.h>

namespace sp {
namespace gui {

SP_REGISTER_WIDGET("image", Image);

Image::Image(P<Widget> parent)
: Widget(parent), uv(0, 0, 1, 1)
{
    loadThemeData("image");
}

void Image::setAttribute(const string& key, const string& value)
{
    if (key == "texture" || key == "image")
    {
        texture = value;
        markRenderDataOutdated();
    }
    else if (key == "uv")
    {
        auto values = value.split(",", 3);
        if (values.size() == 1)
        {
            uv.size.x = uv.size.y = stringutil::convert::toFloat(values[0]);
        }
        else if (values.size() == 2)
        {
            uv.size.x = stringutil::convert::toFloat(values[0]);
            uv.size.y = stringutil::convert::toFloat(values[1]);
        }
        else if (values.size() == 3)
        {
            uv.position.x = uv.position.y = stringutil::convert::toFloat(values[0]);
            uv.size.x = stringutil::convert::toFloat(values[1]);
            uv.size.y = stringutil::convert::toFloat(values[2]);
        }
        else if (values.size() == 4)
        {
            uv.position.x = stringutil::convert::toFloat(values[0]);
            uv.position.y = stringutil::convert::toFloat(values[1]);
            uv.size.x = stringutil::convert::toFloat(values[2]);
            uv.size.y = stringutil::convert::toFloat(values[3]);
        }
        markRenderDataOutdated();
    }
    else
    {
        Widget::setAttribute(key, value);
    }
}

void Image::updateRenderData()
{
    if (texture.length() > 0)
    {
        MeshData::Vertices vertices;
        MeshData::Indices indices{0,1,2, 2,1,3};
        vertices.reserve(4);
        
        Vector2d p0(0, 0);
        Vector2d p1(getRenderSize());
        vertices.emplace_back(Vector3f(p0.x, p0.y, 0.0f), Vector2f(uv.position.x, uv.position.y + uv.size.y));
        vertices.emplace_back(Vector3f(p1.x, p0.y, 0.0f), Vector2f(uv.position.x + uv.size.x, uv.position.y + uv.size.y));
        vertices.emplace_back(Vector3f(p0.x, p1.y, 0.0f), Vector2f(uv.position.x, uv.position.y));
        vertices.emplace_back(Vector3f(p1.x, p1.y, 0.0f), Vector2f(uv.position.x + uv.size.x, uv.position.y));

        render_data.shader = Shader::get("internal:basic.shader");
        render_data.mesh = MeshData::create(std::move(vertices), std::move(indices), MeshData::Type::Dynamic);
        render_data.texture = texture_manager.get(texture);
    }
    else
    {
        render_data.mesh = nullptr;
    }
}

}//namespace gui
}//namespace sp
