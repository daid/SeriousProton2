#include <sp2/graphics/gui/widget/slider.h>
#include <sp2/graphics/gui/widget/themeimage.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/stringutil/convert.h>
#include <sp2/engine.h>


namespace sp {
namespace gui {

SP_REGISTER_WIDGET("slider", Slider);

Slider::Slider(P<Widget> parent)
: Widget(parent)
{
    loadThemeStyle("slider.background");

    min_value = 0.0;
    max_value = 1.0;
    value = 0.5;
    
    dial = new ThemeImage(this, "slider.forground");
    dial->layout.fill_width = false;
    dial->layout.fill_height = false;
    dial->layout.lock_aspect_ratio = true;
    slave_widget = dial;
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
    else if (key == "range")
    {
        if (value.find(",") != -1)
        {
            Vector2f tmp = stringutil::convert::toVector2f(value);
            min_value = tmp.x;
            max_value = tmp.y;
        }
        else
        {
            min_value = 0.0;
            max_value = stringutil::convert::toFloat(value);
        }
        markRenderDataOutdated();
    }
    else if (key == "value")
    {
        this->value = stringutil::convert::toFloat(value);
        markRenderDataOutdated();
    }
    else if (key == "style" || key == "theme_data")
    {
        Widget::setAttribute("style", value + ".background");
        dial->setAttribute("style", value + ".forground");
    }
    else
    {
        Widget::setAttribute(key, value);
    }
}

void Slider::setValue(float value, bool run_callback)
{
    if (max_value < min_value)
        value = std::max(std::min(value, min_value), max_value);
    else
        value = std::max(std::min(value, max_value), min_value);
    if (this->value != value)
    {
        this->value = value;
        markRenderDataOutdated();
        if (run_callback)
            runCallback(value);
    }
}

void Slider::setRange(float min, float max)
{
    min_value = min;
    max_value = max;
    setValue(value);
    markRenderDataOutdated();
}

void Slider::updateRenderData()
{
    Vector2d size(getRenderSize());
    const ThemeStyle::StateStyle& t = theme->states[int(getState())];
    render_data.shader = Shader::get("internal:basic.shader");
    render_data.mesh = createStretched(size);
    render_data.texture = t.texture;
    render_data.color = t.color;
    
    float f = (value - min_value) / (max_value - min_value);
    dial->setVisible(max_value - min_value != 0.0);
        
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
        setValue(new_value);
        runCallback(value);
    }
}

void Slider::onPointerUp(Vector2d position, int id)
{
}

}//namespace gui
}//namespace sp
