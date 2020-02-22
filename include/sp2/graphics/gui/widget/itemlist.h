#ifndef SP2_GRAPHICS_GUI_ITEMLIST_H
#define SP2_GRAPHICS_GUI_ITEMLIST_H

#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/string.h>

namespace sp {
namespace gui {

class ItemList : public Widget
{
public:
    ItemList(P<Widget> parent);

    virtual void setAttribute(const string& key, const string& value) override;
    
    void clearItems();
    int addItem(const string& label);
    int addItem(const string& label, const string& data);

    int getSelectedIndex() const;
    void setSelectedIndex(int index);

    const string& getItemLabel(int index);
    const string& getItemData(int index);
protected:
    struct Item
    {
        string label;
        string data;
    };
    std::vector<Item> items;
    int active_index = 0;
};

}//namespace gui
}//namespace sp

#endif//SP2_GRAPHICS_GUI_ITEMLIST_H
