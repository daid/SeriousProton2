#ifndef SP2_GRAPHICS_GUI_PROGRESSBAR_H
#define SP2_GRAPHICS_GUI_PROGRESSBAR_H

#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/string.h>

namespace sp {
namespace gui {

class Progressbar : public Widget
{
public:
    Progressbar(P<Container> parent);
    
    void setValue(float value);
    void setRange(float min_value, float max_value);
    float getValue() { return value; }
    float getMinRange() { return min_value; }
    float getMaxRange() { return max_value; }

    virtual void setAttribute(const string& key, const string& value) override;
    
    virtual void render(sf::RenderTarget& window) override;
protected:
    Alignment alignment;
    float value;
    float min_value;
    float max_value;
    
    virtual sp::Color getProgressColor();
};

};//!namespace gui
};//!namespace sp

#endif//SP2_GRAPHICS_GUI_PROGRESSBAR_H

