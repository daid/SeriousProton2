#ifndef SP2_GRAPHICS_GUI_LAYOUT_H
#define SP2_GRAPHICS_GUI_LAYOUT_H

#include <sp2/string.h>
#include <sp2/pointerVector.h>
#include <sp2/graphics/gui/container.h>
#include <SFML/Graphics/Rect.hpp>

namespace sp {
namespace gui {

class Layout : sf::NonCopyable
{
public:
    virtual ~Layout() {}
    virtual void update(P<Container> container, sf::FloatRect& rect);

protected:
    virtual void basicLayout(const sf::FloatRect& rect, Widget* widget);
};

class LayoutClassRegistry : sf::NonCopyable
{
public:
    static LayoutClassRegistry* first;
    
    LayoutClassRegistry* next;
    string name;
    std::function<Layout*()> creation_function;
    
    LayoutClassRegistry(string name, std::function<Layout*()> creation_function)
    : name(name), creation_function(creation_function)
    {
        next = first;
        first = this;
    }
};

#define SP_REGISTER_LAYOUT(name, class_name) \
    sp::gui::LayoutClassRegistry layout_class_registry ## class_name (name, []() { return new class_name(); });

};//!namespace gui
};//!namespace sp

#endif//SP2_GRAPHICS_GUI_LAYOUT_H
