#ifndef SP2_GRAPHICS_GUI_TEXTFIELD_H
#define SP2_GRAPHICS_GUI_TEXTFIELD_H

#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/string.h>

namespace sp {
namespace gui {

class TextField : public Widget
{
public:
    TextField(P<Widget> parent);

    virtual void setAttribute(const string& key, const string& value) override;
    
    virtual void updateRenderData() override;
    virtual void onUpdate(float delta) override;
    virtual bool onPointerDown(io::Pointer::Button button, Vector2d position, int id) override;
    virtual void onPointerDrag(Vector2d position, int id) override;
    virtual void onPointerUp(Vector2d position, int id) override;
    virtual void onTextInput(const string& text) override;
    virtual void onTextInput(TextInputEvent e) override;

    const string& getValue() { return value; }
private:
    float text_size;
    int texture_revision;
    
    string value;
    int cursor;
};

};//namespace gui
};//namespace sp

#endif//SP2_GRAPHICS_GUI_TEXTFIELD_H

