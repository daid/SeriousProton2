#ifndef SP2_GRAPHICS_GUI_IMAGE_H
#define SP2_GRAPHICS_GUI_IMAGE_H

#include <sp2/graphics/gui/widget/widget.h>

namespace sp {
namespace gui {

class Image : public Widget
{
public:
    Image(P<Container> parent);
    
    virtual void setAttribute(const string& key, const string& value) override;
    
    virtual void render(sf::RenderTarget& window) override;
private:
    string texture;
};

};//!namespace gui
};//!namespace sp

#endif//SP2_GRAPHICS_GUI_IMAGE_H

