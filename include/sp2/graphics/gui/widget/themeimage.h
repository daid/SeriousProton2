#ifndef SP2_GRAPHICS_GUI_THEMEIMAGE_H
#define SP2_GRAPHICS_GUI_THEMEIMAGE_H

#include <sp2/graphics/gui/widget/widget.h>

namespace sp {
namespace gui {

class ThemeImage : public Widget
{
public:
    enum class Orientation
    {
        Auto,
        Horizontal,
        Vertical
    };

    ThemeImage(P<Widget> parent, const string& theme_style_name);
    
    void setOrientation(Orientation orientation);
    
    virtual void updateRenderData() override;

private:
    Orientation orientation;
};

}//namespace gui
}//namespace sp

#endif//SP2_GRAPHICS_GUI_IMAGE_H
