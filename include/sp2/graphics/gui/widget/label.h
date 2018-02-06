#ifndef SP2_GRAPHICS_GUI_LABEL_H
#define SP2_GRAPHICS_GUI_LABEL_H

#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/string.h>

namespace sp {
namespace gui {

class Label : public Widget
{
public:
    Label(P<Widget> parent, string theme_data_name="label");

    void setLabel(string label);
    
    Alignment getTextAlignment() { return text_alignment; }
    void setTextAlignment(Alignment alignment) { text_alignment = alignment; }
    
    virtual void setAttribute(const string& key, const string& value) override;
    
    virtual void updateRenderData() override;
private:
    Alignment text_alignment;
    string label;
    float text_size;
    bool vertical;
};

};//!namespace gui
};//!namespace sp

#endif//SP2_GRAPHICS_GUI_LABEL_H

