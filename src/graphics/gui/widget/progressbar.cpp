#include <sp2/graphics/gui/widget/progressbar.h>
#include <sp2/graphics/gui/widget/themeimage.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/stringutil/convert.h>
#include <sp2/engine.h>


namespace sp {
namespace gui {

Progressbar::Progressbar(P<Widget> parent)
: Widget(parent)
{
    loadThemeStyle("progressbar.background");
    value = 0.5;
    min_value = 0.0;
    max_value = 1.0;
    
    fill = new ThemeImage(this, "progressbar.forground");
    fill->layout.alignment = Alignment::Left;
    slave_widget = fill;
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
    else if (key == "style" || key == "theme_data")
    {
        Widget::setAttribute("style", value + ".background");
        fill->setAttribute("style", value + ".forground");
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

    updateRenderDataToThemeImage();

    fill->layout.size = getRenderSize();
    switch(fill->layout.alignment)
    {
    case Alignment::TopLeft:
    case Alignment::Top:
    case Alignment::TopRight:
    case Alignment::BottomLeft:
    case Alignment::Bottom:
    case Alignment::BottomRight:
        fill->layout.size.y *= f;
        fill->setFlags(render_flag_vertical_corner_clip);
        break;
    case Alignment::Left:
    case Alignment::Right:
    case Alignment::Center:
        fill->layout.size.x *= f;
        fill->setFlags(render_flag_horizontal_corner_clip);
        break;
    }
}

}//namespace gui
}//namespace sp
