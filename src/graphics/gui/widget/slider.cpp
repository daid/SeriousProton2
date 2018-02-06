#include <sp2/graphics/gui/widget/slider.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/engine.h>
#include <SFML/Graphics.hpp>

namespace sp {
namespace gui {

SP_REGISTER_WIDGET("slider", Slider);

Slider::Slider(P<Widget> parent)
: Widget(parent)
{
    loadThemeData("slider");

    min_value = 0.0;
    max_value = 1.0;
    value = 0.5;
}

void Slider::setAttribute(const string& key, const string& value)
{
    if (key == "min" || key == "min_value")
    {
        min_value = stringutil::convert::toFloat(value);
        markRenderDataOutdated();
    }
    else if (key == "max" || key == "max_value")
    {
        max_value = stringutil::convert::toFloat(value);
        markRenderDataOutdated();
    }
    else if (key == "value")
    {
        this->value = stringutil::convert::toFloat(value);
        markRenderDataOutdated();
    }
    else
    {
        Widget::setAttribute(key, value);
    }
}

void Slider::updateRenderData()
{
    const ThemeData::StateData& t = theme->states[int(getState())];
    render_data.mesh = createStretched(getRenderSize());
    render_data.texture = t.texture;
    render_data.color = t.color;
    /*TODO:GUI
    sf::FloatRect rect(getRect().left, getRect().top, std::min(getRect().width, getRect().height), std::min(getRect().width, getRect().height));
    
    float f = (value - min_value) / (max_value - min_value);
    if (getRect().width > getRect().height)
    {
        rect.left += (getRect().width - getRect().height) * f;
    }
    else
    {
        rect.top += (getRect().height - getRect().width) * f;
    }
    //renderStretched(window, rect, t.forground_image, t.forground_color);
    */
}

bool Slider::onPointerDown(io::Pointer::Button button, Vector2d position, int id)
{
    onPointerDrag(position, id);
    return true;
}

void Slider::onPointerDrag(Vector2d position, int id)
{
    float f;
    Vector2d size = getRenderSize();
    if (size.x > size.y)
        f = (position.x - size.y / 2.0) / (size.x - size.y);
    else
        f = (position.y - size.x / 2.0) / (size.y - size.x);
    f = std::max(0.0f, std::min(f, 1.0f));
    value = (f * (max_value - min_value)) + min_value;
    
    runCallback(value);
}

void Slider::onPointerUp(Vector2d position, int id)
{
}

};//!namespace gui
};//!namespace sp
