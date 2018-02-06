#include <sp2/graphics/gui/widget/progressbar.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/engine.h>
#include <SFML/Graphics.hpp>

namespace sp {
namespace gui {

SP_REGISTER_WIDGET("progressbar", Progressbar);

Progressbar::Progressbar(P<Widget> parent)
: Widget(parent)
{
    loadThemeData("progressbar");
    alignment = Alignment::Left;
    value = 0.5;
    min_value = 0.0;
    max_value = 1.0;
}

void Progressbar::setValue(float value)
{
    if (this->value != value)
    {
        this->value = value;
        markRenderDataOutdated();
    }
}

void Progressbar::setRange(float min_value, float max_value)
{
    if (this->min_value != min_value || this->max_value != max_value)
    {
        this->min_value = min_value;
        this->max_value = max_value;
        markRenderDataOutdated();
    }
}

void Progressbar::setAttribute(const string& key, const string& value)
{
    if (key == "progress_alignment")
    {
        if (value == "topleft") alignment = Alignment::TopLeft;
        else if (value == "top") alignment = Alignment::Top;
        else if (value == "topright") alignment = Alignment::TopRight;
        else if (value == "left") alignment = Alignment::Left;
        else if (value == "center") alignment = Alignment::Center;
        else if (value == "right") alignment = Alignment::Right;
        else if (value == "bottomleft") alignment = Alignment::BottomLeft;
        else if (value == "bottom") alignment = Alignment::Bottom;
        else if (value == "bottomright") alignment = Alignment::BottomRight;
        markRenderDataOutdated();
    }
    else if (key == "range")
    {
        sp::Vector2f v = stringutil::convert::toVector2f(value);
        max_value = v.y;
        if (v.x != v.y)
            min_value = v.x;
        markRenderDataOutdated();
    }
    else
    {
        Widget::setAttribute(key, value);
    }
}

void Progressbar::updateRenderData()
{
    float f = (value - min_value) / (max_value - min_value);
    f = std::max(0.0f, std::min(f, 1.0f));
    const ThemeData::StateData& t = theme->states[int(getState())];
    render_data.shader = Shader::get("internal:basic.shader");
    render_data.texture = t.texture;
    render_data.color = t.color;
    //TODO:GUI
    switch(alignment)
    {
    case Alignment::TopLeft:
    case Alignment::Top:
    case Alignment::TopRight:
        render_data.mesh = createStretchedV(getRenderSize());
        //rect.height *= f;
        //renderStretchedV(window, rect, t.forground_image, getProgressColor());
        break;
    case Alignment::BottomLeft:
    case Alignment::Bottom:
    case Alignment::BottomRight:
        render_data.mesh = createStretchedV(getRenderSize());
        //renderStretchedV(window, rect, t.background_image, t.background_color);
        //rect.top += rect.height * (1.0 - f);
        //rect.height *= f;
        //renderStretchedV(window, rect, t.forground_image, getProgressColor());
        break;
    case Alignment::Left:
        render_data.mesh = createStretchedH(getRenderSize());
        //renderStretchedH(window, rect, t.background_image, t.background_color);
        //rect.width *= f;
        //renderStretchedH(window, rect, t.forground_image, getProgressColor());
        break;
    case Alignment::Right:
        render_data.mesh = createStretchedH(getRenderSize());
        //renderStretchedH(window, rect, t.background_image, t.background_color);
        //rect.left += rect.width * (1.0 - f);
        //rect.width *= f;
        //renderStretchedH(window, rect, t.forground_image, getProgressColor());
        break;
    case Alignment::Center:
        render_data.mesh = createStretchedH(getRenderSize());
        //renderStretchedH(window, rect, t.background_image, t.background_color);
        //rect.left += rect.width * (1.0 - f) * 0.5;
        //rect.width *= f;
        //renderStretchedH(window, rect, t.forground_image, getProgressColor());
        break;
    }
}

};//!namespace gui
};//!namespace sp
