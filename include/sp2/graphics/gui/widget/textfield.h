#ifndef SP2_GRAPHICS_GUI_TEXTFIELD_H
#define SP2_GRAPHICS_GUI_TEXTFIELD_H

#include <sp2/graphics/gui/widget/textarea.h>
#include <sp2/string.h>

namespace sp {
namespace gui {

class TextField : public TextArea
{
public:
    TextField(P<Widget> parent);
};

}//namespace gui
}//namespace sp

#endif//SP2_GRAPHICS_GUI_TEXTFIELD_H

