#ifndef SP2_GRAPHICS_GUI_IMAGE_H
#define SP2_GRAPHICS_GUI_IMAGE_H

#include <sp2/math/rect.h>
#include <sp2/graphics/gui/widget/widget.h>

namespace sp {
namespace gui {

class Image : public Widget
{
public:
    Image(P<Widget> parent);
    
    virtual void setAttribute(const string& key, const string& value) override;
    
    virtual void updateRenderData() override;
    void setUV(sp::Rect2d uv_rect) { uv = uv_rect; }
private:
    string texture;
    Rect2d uv;
    Vector2d tile_size;
};

}//namespace gui
}//namespace sp

#endif//SP2_GRAPHICS_GUI_IMAGE_H

