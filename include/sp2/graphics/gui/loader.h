#ifndef SP2_GRAPHICS_GUI_LOADER_H
#define SP2_GRAPHICS_GUI_LOADER_H

#include <sp2/string.h>
#include <sp2/pointer.h>

namespace sp {
class KeyValueTree;
class KeyValueTreeNode;

namespace gui {

class Widget;

class Loader : NonCopyable
{
public:
    static P<Widget> load(string resource_name, string root_id, P<Widget> root=nullptr, bool auto_reload=false);

private:
    P<KeyValueTree> tree;
    P<Widget> createWidget(P<Widget> parent, KeyValueTreeNode& node, std::map<string, string>& parameters);
    void loadWidgetFromTree(P<Widget> widget, KeyValueTreeNode& node, std::map<string, string>& parameters);
    string getType(KeyValueTreeNode& node, std::map<string, string>& parameters);
};

};//!namespace gui
};//!namespace sp

#endif//SP2_GRAPHICS_GUI_LOADER_H
