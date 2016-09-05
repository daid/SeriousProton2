#ifndef SP2_GRAPHICS_GUI_THEME_H
#define SP2_GRAPHICS_GUI_THEME_H

#include <sp2/pointerVector.h>
#include <sp2/string.h>
#include <sp2/graphics/gui/widget/widget.h>
#include <SFML/Graphics/Color.hpp>
#include <map>

namespace sp {
namespace gui {

/**
    A theme is used to style different widgets.
    A single theme contains information on how to style different widget elements.
    
    Each element describes the the following properties:
    * background image
    * background color
    * font
    * text size
    * text color
    With the possibility to distingish with the following states:
    * normal
    * disabled
    * focused
    * hover
**/
class ThemeData
{
    class StateData
    {
    public:
        string background_image;
        sf::Color background_color;
        sf::Color forground_color;

        string font;
        float text_size;
    };
    StateData states[int(Widget::State::Count)];
};

class Theme : public AutoPointerObject
{
public:
    const ThemeData* getData(string element);

    static P<Theme> getTheme(string name);
private:
    Theme(string name);
    virtual ~Theme();

    string name;
    std::map<string, ThemeData> data;
    
    static std::map<string, P<Theme>> themes;
};

};//!namespace gui
};//!namespace sp

#endif//SP2_GRAPHICS_GUI_THEME_H
