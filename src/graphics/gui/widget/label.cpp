#include <sp2/graphics/gui/widget/label.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/engine.h>
#include <SFML/Graphics.hpp>

namespace sp {
namespace gui {

SP_REGISTER_WIDGET("label", Label);

Label::Label(P<Container> parent)
: Widget(parent)
{
    loadThemeData("label");
    text_alignment = Alignment::Center;
    vertical = false;
}

void Label::setLabel(string label)
{
    this->label = label;
}

void Label::setAttribute(const string& key, const string& value)
{
    if (key == "label" || key == "caption")
    {
        label = value;
    }
    else if (key == "text_alignment")
    {
        if (value == "topleft") text_alignment = Alignment::TopLeft;
        else if (value == "top") text_alignment = Alignment::Top;
        else if (value == "right") text_alignment = Alignment::TopRight;
        else if (value == "left") text_alignment = Alignment::Left;
        else if (value == "center") text_alignment = Alignment::Center;
        else if (value == "right") text_alignment = Alignment::Right;
        else if (value == "bottomleft") text_alignment = Alignment::BottomLeft;
        else if (value == "bottom") text_alignment = Alignment::Bottom;
        else if (value == "bottomright") text_alignment = Alignment::BottomRight;
    }
    else if (key == "vertical")
    {
        if (value == "true")
            vertical = true;
        else
            vertical = value.toInt();
    }
    else
    {
        Widget::setAttribute(key, value);
    }
}

void Label::render(sf::RenderTarget& window)
{
    const ThemeData::StateData& t = theme->states[int(getState())];
    //renderStretched(window, layout.rect, t.background_image, t.background_color);
    if (vertical)
        renderTextVertical(window, layout.rect, text_alignment, label, t.font, t.text_size, t.forground_color);
    else
        renderText(window, layout.rect, text_alignment, label, t.font, t.text_size, t.forground_color);
}

};//!namespace gui
};//!namespace sp
