#include <sp2/graphics/gui/widget/image.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/engine.h>

namespace sp {
namespace gui {

SP_REGISTER_WIDGET("image", Image);

Image::Image(P<Widget> parent)
: Widget(parent)
{
    loadThemeData("image");
}

void Image::setAttribute(const string& key, const string& value)
{
    if (key == "texture")
    {
        texture = value;
        markRenderDataOutdated();
    }
    else
    {
        Widget::setAttribute(key, value);
    }
}

void Image::updateRenderData()
{
    MeshData::Vertices vertices;
    MeshData::Indices indices{0,1,2, 2,1,3};
    vertices.reserve(4);
    
    Vector2d p0(0, 0);
    Vector2d p1(getRenderSize());
    vertices.emplace_back(Vector3f(p0.x, p0.y, 0.0f), Vector2f(0, 1));
    vertices.emplace_back(Vector3f(p1.x, p0.y, 0.0f), Vector2f(1, 1));
    vertices.emplace_back(Vector3f(p0.x, p1.y, 0.0f), Vector2f(0, 0));
    vertices.emplace_back(Vector3f(p1.x, p1.y, 0.0f), Vector2f(1, 0));

    render_data.shader = Shader::get("internal:basic.shader");
    render_data.mesh = MeshData::create(std::move(vertices), std::move(indices), MeshData::Type::Dynamic);
    render_data.texture = textureManager.get(texture);
}

};//!namespace gui
};//!namespace sp
