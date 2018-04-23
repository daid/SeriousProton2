#ifndef SP2_GRAPHICS_GUI_THEMEIMAGE_H
#define SP2_GRAPHICS_GUI_THEMEIMAGE_H

#include <sp2/graphics/gui/widget/widget.h>

namespace sp {
namespace gui {

class ThemeImage : public Widget
{
public:
    ThemeImage(P<Widget> parent, string theme_data_name);
    
    virtual void updateRenderData() override;
};

};//namespace gui
};//namespace sp

#endif//SP2_GRAPHICS_GUI_IMAGE_H
