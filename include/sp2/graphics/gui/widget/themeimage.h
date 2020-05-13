#ifndef SP2_GRAPHICS_GUI_THEMEIMAGE_H
#define SP2_GRAPHICS_GUI_THEMEIMAGE_H

#include <sp2/graphics/gui/widget/widget.h>

namespace sp {
namespace gui {

class ThemeImage : public Widget
{
public:
    ThemeImage(P<Widget> parent, const string& theme_style_name);

    void setFlags(int flags);

    virtual void updateRenderData() override;

private:
    int flags = 0;
};

}//namespace gui
}//namespace sp

#endif//SP2_GRAPHICS_GUI_IMAGE_H
