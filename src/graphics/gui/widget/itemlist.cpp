#include <sp2/graphics/gui/widget/itemlist.h>


namespace sp {
namespace gui {

ItemList::ItemList(P<Widget> parent)
: Widget(parent)
{
}

void ItemList::setAttribute(const string& key, const string& value)
{
    if (key == "items")
    {
        for(const auto& e : value.split(","))
            addItem(e.strip());
    }
    else
    {
        Widget::setAttribute(key, value);
    }
}

void ItemList::clearItems()
{
    items.clear();
    active_index = 0;
    markRenderDataOutdated();
}

int ItemList::addItem(const string& label)
{
    items.push_back({label, ""});
    markRenderDataOutdated();
    return items.size() - 1;
}

int ItemList::addItem(const string& label, const string& data)
{
    items.push_back({label, data});
    markRenderDataOutdated();
    return items.size() - 1;
}

int ItemList::getSelectedIndex() const
{
    return active_index;
}

void ItemList::setSelectedIndex(int index)
{
    if (items.size() > 0)
    {
        while(index < 0)
            index += items.size();
        active_index = index % items.size();
    }
    else
    {
        active_index = 0;
    }
    markRenderDataOutdated();
}

string ItemList::getItemLabel(int index)
{
    if (index < 0 || index >= int(items.size()))
        return "";
    return items[index].label;
}

string ItemList::getItemData(int index)
{
    if (index < 0 || index >= int(items.size()))
        return "";
    return items[index].data;
}

}//namespace gui
}//namespace sp
