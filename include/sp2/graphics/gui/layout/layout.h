#ifndef SP2_GRAPHICS_GUI_LAYOUT_H
#define SP2_GRAPHICS_GUI_LAYOUT_H

#include <sp2/string.h>
#include <sp2/math/rect.h>
#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/attributes.h>


namespace sp {
namespace gui {

class Layout : NonCopyable
{
public:
    virtual ~Layout() {}
    void updateLoop(P<Widget> container, Rect2d rect);
    virtual void update(P<Widget> container, Rect2d rect);

protected:
    virtual void basicLayout(Rect2d rect, Widget* widget);

private:
    bool require_repeat = false;
};

class LayoutClassRegistry : NonCopyable
{
public:
    static LayoutClassRegistry* first;
    
    LayoutClassRegistry* next;
    string name;
    std::function<Layout*()> creation_function;
    
    LayoutClassRegistry(const string& name, std::function<Layout*()> creation_function)
    : name(name), creation_function(creation_function)
    {
        next = first;
        first = this;
    }
};

#define SP_REGISTER_LAYOUT(name, class_name) \
    sp::gui::LayoutClassRegistry SP2_STATIC_INIT layout_class_registry ## class_name (name, []() { return new class_name(); });

}//namespace gui
}//namespace sp

#endif//SP2_GRAPHICS_GUI_LAYOUT_H
