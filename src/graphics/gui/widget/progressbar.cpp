#include <sp2/graphics/gui/widget/progressbar.h>
#include <sp2/graphics/gui/widget/themeimage.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/engine.h>


namespace sp {
namespace gui {

SP_REGISTER_WIDGET("progressbar", Progressbar);

Progressbar::Progressbar(P<Widget> parent)
: Widget(parent)
{
    loadThemeData("progressbar.background");
    value = 0.5;
    min_value = 0.0;
    max_value = 1.0;
    
    fill = new ThemeImage(this, "progressbar.forground");
    fill->layout.alignment = Alignment::Left;
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
    if (key == "progress_alignment" || key == "progress.alignment")
    {
        fill->setAttribute("alignment", value);
        markRenderDataOutdated();
    }
    else if (key == "value")
    {
        this->value = stringutil::convert::toFloat(value);
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
    else if (key == "theme_data")
    {
        fill->setAttribute(key, value);
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

    fill->layout.size = getRenderSize();
    switch(fill->layout.alignment)
    {
    case Alignment::TopLeft:
    case Alignment::Top:
    case Alignment::TopRight:
        render_data.mesh = createStretchedV(getRenderSize());
        fill->layout.size.y *= f;
        fill->setOrientation(ThemeImage::Orientation::Vertical);
        break;
    case Alignment::BottomLeft:
    case Alignment::Bottom:
    case Alignment::BottomRight:
        render_data.mesh = createStretchedV(getRenderSize());
        fill->layout.size.y *= f;
        fill->setOrientation(ThemeImage::Orientation::Vertical);
        break;
    case Alignment::Left:
    case Alignment::Right:
    case Alignment::Center:
        render_data.mesh = createStretchedH(getRenderSize());
        fill->layout.size.x *= f;
        fill->setOrientation(ThemeImage::Orientation::Horizontal);
        break;
    }
}

};//namespace gui
};//namespace sp
