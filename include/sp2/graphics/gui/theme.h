#ifndef SP2_GRAPHICS_GUI_THEME_H
#define SP2_GRAPHICS_GUI_THEME_H

#include <sp2/string.h>
#include <sp2/graphics/gui/widget/widget.h>
#include <map>

namespace sp {
class Font;
namespace gui {

class ThemeData
{
public:
    class StateData
    {
    public:
        Texture* texture;
        sp::Color color;
        float size; //general size parameter, depends on the widget type what it means.
        Font* font;
    };
    StateData states[int(Widget::State::Count)];
};

/** The Theme class is used by the sp::gui::widget classes to style themselves.
    
    Themes are loaded from a text resource, and referenced from sp::gui::widget classes.

    A single theme contains information on how to style different widget elements.
    
    Each element describes the the following properties:
    - texture
    - color
    - font
    - size
   
    With the possibility to distingish with the following states:
    - normal: Default state
    - disabled: When enabled is false
    - focused: When this gui element has keyboard focus (last clicked)
    - hover: When the mouse pointer is on top of it
**/
class Theme : public AutoPointerObject
{
public:
    const ThemeData* getData(string element);

    static P<Theme> getTheme(string name);
    static void loadTheme(string name, string resource_name);
private:
    Theme(string name);
    virtual ~Theme();

    string name;
    std::map<string, ThemeData> data;
    
    static std::map<string, P<Theme>> themes;
};

}//namespace gui
}//namespace sp

#endif//SP2_GRAPHICS_GUI_THEME_H
