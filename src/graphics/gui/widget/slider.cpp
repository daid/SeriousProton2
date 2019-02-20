#include <sp2/graphics/gui/widget/slider.h>
#include <sp2/graphics/gui/widget/themeimage.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/engine.h>

namespace sp {
namespace gui {

SP_REGISTER_WIDGET("slider", Slider);

Slider::Slider(P<Widget> parent)
: Widget(parent)
{
    loadThemeData("slider.background");

    min_value = 0.0;
    max_value = 1.0;
    value = 0.5;
    
    dial = new ThemeImage(this, "slider.forground");
    dial->layout.fill_width = false;
    dial->layout.fill_height = false;
    dial->layout.lock_aspect_ratio = true;
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

void Slider::setValue(float value)
{
    this->value = value;
    markRenderDataOutdated();
}

void Slider::setRange(float min, float max)
{
    min_value = min;
    max_value = max;
    markRenderDataOutdated();
}

void Slider::updateRenderData()
{
    Vector2d size(getRenderSize());
    const ThemeData::StateData& t = theme->states[int(getState())];
    render_data.shader = Shader::get("internal:basic.shader");
    render_data.mesh = createStretched(size);
    render_data.texture = t.texture;
    render_data.color = t.color;
    
    float f = (value - min_value) / (max_value - min_value);
    if (max_value - min_value == 0.0)
        f = 0.5;
    if (size.x > size.y)
    {
        dial->layout.size.x = size.y;
        dial->layout.size.y = size.y;
        dial->layout.position.x = (size.x - size.y) * f;
        dial->layout.position.y = 0;
        dial->layout.fill_width = false;
        dial->layout.fill_height = true;
    }
    else
    {
        dial->layout.size.x = size.x;
        dial->layout.size.y = size.x;
        dial->layout.position.x = 0;
        dial->layout.position.y = size.y - size.x - (size.y - size.x) * f;
        dial->layout.fill_width = true;
        dial->layout.fill_height = false;
    }
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
    float new_value = (f * (max_value - min_value)) + min_value;
    if (new_value != value)
    {
        value = new_value;
        runCallback(value);
        markRenderDataOutdated();
    }
}

void Slider::onPointerUp(Vector2d position, int id)
{
}

};//namespace gui
};//namespace sp
