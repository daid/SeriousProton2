#ifndef SP2_GRAPHICS_GUI_TEXTAREA_H
#define SP2_GRAPHICS_GUI_TEXTAREA_H

#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/string.h>

namespace sp {
namespace gui {

class Slider;
class TextArea : public Widget
{
public:
    TextArea(P<Widget> parent);

    virtual void setAttribute(const string& key, const string& value) override;

    virtual void updateRenderData() override;
    virtual void onUpdate(float delta) override;
    virtual bool onPointerDown(io::Pointer::Button button, Vector2d position, int id) override;
    virtual void onPointerDrag(Vector2d position, int id) override;
    virtual void onPointerUp(Vector2d position, int id) override;
    virtual bool enableTextInputOnFocus() const override { return true; }
    virtual void onTextInput(const string& text) override;
    virtual void onTextInput(TextInputEvent e) override;

    const string& getValue() const;
private:
    int getTextOffsetForPosition(Vector2d position);

    sp::P<Slider> vertical_scroll;
    sp::P<Widget> cursor_widget;
    float text_size;
    bool multiline = true;
    bool readonly = false;
    int texture_revision;

    string value;
    int selection_start = 0;
    int selection_end = 0;
};

}//namespace gui
}//namespace sp

#endif//SP2_GRAPHICS_GUI_TEXTAREA_H

