#include <sp2/graphics/gui/widget/progressbar.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/engine.h>
#include <SFML/Graphics.hpp>

namespace sp {
namespace gui {

SP_REGISTER_WIDGET("progressbar", Progressbar);

Progressbar::Progressbar(P<Container> parent)
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
    this->value = value;
}

void Progressbar::setRange(float min_value, float max_value)
{
    this->min_value = min_value;
    this->max_value = max_value;
}

void Progressbar::setAttribute(const string& key, const string& value)
{
    if (key == "progress_alignment")
    {
        if (value == "topleft") alignment = Alignment::TopLeft;
        else if (value == "top") alignment = Alignment::Top;
        else if (value == "right") alignment = Alignment::TopRight;
        else if (value == "left") alignment = Alignment::Left;
        else if (value == "center") alignment = Alignment::Center;
        else if (value == "right") alignment = Alignment::Right;
        else if (value == "bottomleft") alignment = Alignment::BottomLeft;
        else if (value == "bottom") alignment = Alignment::Bottom;
        else if (value == "bottomright") alignment = Alignment::BottomRight;
    }
    else
    {
        Widget::setAttribute(key, value);
    }
}

void Progressbar::render(sf::RenderTarget& window)
{
    float f = (value - min_value) / (max_value - min_value);
    f = std::max(0.0f, std::min(f, 1.0f));

    const ThemeData::StateData& t = theme->states[int(getState())];
    sf::FloatRect rect = layout.rect;
    
    switch(alignment)
    {
    case Alignment::TopLeft:
    case Alignment::Top:
    case Alignment::TopRight:
        renderStretchedV(window, rect, t.background_image, t.background_color);
        rect.height *= f;
        renderStretchedV(window, rect, t.forground_image, t.forground_color);
        break;
    case Alignment::BottomLeft:
    case Alignment::Bottom:
    case Alignment::BottomRight:
        renderStretchedV(window, rect, t.background_image, t.background_color);
        rect.top += rect.height * (1.0 - f);
        rect.height *= f;
        renderStretchedV(window, rect, t.forground_image, t.forground_color);
        break;
    case Alignment::Left:
        renderStretchedH(window, rect, t.background_image, t.background_color);
        rect.width *= f;
        renderStretchedH(window, rect, t.forground_image, t.forground_color);
        break;
    case Alignment::Right:
        renderStretchedH(window, rect, t.background_image, t.background_color);
        rect.left += rect.width * (1.0 - f);
        rect.width *= f;
        renderStretchedH(window, rect, t.forground_image, t.forground_color);
        break;
    case Alignment::Center:
        renderStretchedH(window, rect, t.background_image, t.background_color);
        rect.left += rect.width * (1.0 - f) * 0.5;
        rect.width *= f;
        renderStretchedH(window, rect, t.forground_image, t.forground_color);
        break;
    }
}

};//!namespace gui
};//!namespace sp
