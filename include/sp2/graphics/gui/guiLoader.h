#ifndef SP2_GRAPHICS_GUI_LOADER_H
#define SP2_GRAPHICS_GUI_LOADER_H

#include <sp2/string.h>
#include <sp2/pointer.h>
#include <SFML/System/NonCopyable.hpp>

namespace sp {
class KeyValueTree;
class KeyValueTreeNode;

namespace gui {

class Widget;

class Loader : sf::NonCopyable
{
public:
    static P<Widget> load(string resource_name, string root_id, P<Widget> root, bool auto_reload=false);

private:
    P<KeyValueTree> tree;
    P<Widget> createWidget(P<Widget> parent, KeyValueTreeNode& node);
    void loadWidgetFromTree(P<Widget> widget, KeyValueTreeNode& node);
    string getType(KeyValueTreeNode& node);
};

};//!namespace gui
};//!namespace sp

#endif//SP2_GRAPHICS_GUI_LOADER_H
