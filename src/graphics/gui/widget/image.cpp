#include <sp2/graphics/gui/widget/image.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/stringutil/convert.h>
#include <sp2/engine.h>

namespace sp {
namespace gui {

Image::Image(P<Widget> parent)
: Widget(parent), uv(0, 0, 1, 1)
{
    loadThemeStyle("image");
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
    else if (key == "tile")
    {
        tile_size = stringutil::convert::toVector2d(value);
    }
    else if (key == "color")
    {
        render_data.color = stringutil::convert::toColor(value);
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
        if (tile_size.x <= 0.0 || tile_size.y <= 0.0)
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

            render_data.mesh = MeshData::create(std::move(vertices), std::move(indices), MeshData::Type::Dynamic);
        }
        else
        {
            MeshData::Vertices vertices;
            MeshData::Indices indices;
            int count = std::ceil(getRenderSize().x / tile_size.x) * std::ceil(getRenderSize().y / tile_size.y);
            vertices.reserve(4 * count);
            indices.reserve(6 * count);

            for(float y0=0.0f; y0<getRenderSize().y; y0+=tile_size.y)
            {
                for(float x0=0.0f; x0<getRenderSize().x; x0+=tile_size.x)
                {
                    float x1 = std::min(x0 + tile_size.x, getRenderSize().x);
                    float y1 = std::min(y0 + tile_size.y, getRenderSize().y);
                    float u = uv.size.x * (x1 - x0) / tile_size.x;
                    float v = uv.size.y * (y1 - y0) / tile_size.y;
                    uint16_t idx = vertices.size();
                    vertices.emplace_back(Vector3f(x0, y0, 0.0f), Vector2f(uv.position.x, uv.position.y + v));
                    vertices.emplace_back(Vector3f(x1, y0, 0.0f), Vector2f(uv.position.x + u, uv.position.y + v));
                    vertices.emplace_back(Vector3f(x0, y1, 0.0f), Vector2f(uv.position.x, uv.position.y));
                    vertices.emplace_back(Vector3f(x1, y1, 0.0f), Vector2f(uv.position.x + u, uv.position.y));
                    indices.push_back(idx+0);
                    indices.push_back(idx+1);
                    indices.push_back(idx+2);
                    indices.push_back(idx+2);
                    indices.push_back(idx+1);
                    indices.push_back(idx+3);
                }
            }

            render_data.mesh = MeshData::create(std::move(vertices), std::move(indices), MeshData::Type::Dynamic);
        }

        render_data.shader = Shader::get("internal:basic.shader");
        render_data.texture = texture_manager.get(texture);
    }
    else
    {
        render_data.mesh = nullptr;
    }
}

}//namespace gui
}//namespace sp
