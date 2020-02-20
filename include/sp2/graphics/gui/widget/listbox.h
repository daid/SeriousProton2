#ifndef SP2_GRAPHICS_GUI_LISTBOX_H
#define SP2_GRAPHICS_GUI_LISTBOX_H

#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/string.h>

namespace sp {
namespace gui {

class Slider;
class Listbox : public Widget
{
public:
    Listbox(P<Widget> parent);

    virtual void setAttribute(const string& key, const string& value) override;
    
    virtual void onUpdate(float delta) override;
    virtual void updateRenderData() override;
    virtual bool onPointerDown(io::Pointer::Button button, Vector2d position, int id) override;
    virtual void onPointerDrag(Vector2d position, int id) override;
    virtual void onPointerUp(Vector2d position, int id) override;
    
    void clearItems();
    int addItem(const string& label);

    int getSelectedIndex() const;
    void setSelectedIndex(int index);
private:
    sp::P<Slider> slider;

    std::unordered_map<int, Vector2d> pointer_position;
    const ThemeStyle* text_theme;
    float text_size = 0.0f;
    sp::PList<sp::Node> text_nodes;
    std::vector<string> items;
    int active_index = 0;
};

}//namespace gui
}//namespace sp

#endif//SP2_GRAPHICS_GUI_LISTBOX_H
