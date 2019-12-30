#ifndef SP2_GRAPHICS_GUI_LABEL_H
#define SP2_GRAPHICS_GUI_LABEL_H

#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/string.h>

namespace sp {
namespace gui {

class Label : public Widget
{
public:
    Label(P<Widget> parent, const string& theme_data_name="label");

    void setLabel(const string& label);
    const string& getLabel();
    
    Alignment getTextAlignment() { return text_alignment; }
    void setTextAlignment(Alignment alignment) { text_alignment = alignment; }
    
    virtual void setAttribute(const string& key, const string& value) override;
    
    virtual void updateRenderData() override;
    virtual void onUpdate(float delta) override;
private:
    Alignment text_alignment;
    string label;
    float text_size;
    bool vertical;
    bool scale_to_text = false;
    int texture_revision;
};

}//namespace gui
}//namespace sp

#endif//SP2_GRAPHICS_GUI_LABEL_H

