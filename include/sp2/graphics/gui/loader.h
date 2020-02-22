#ifndef SP2_GRAPHICS_GUI_LOADER_H
#define SP2_GRAPHICS_GUI_LOADER_H

#include <sp2/string.h>
#include <sp2/pointer.h>
#include <sp2/keyValueTree.h>
#include <sp2/graphics/gui/widget/widget.h>

namespace sp {
namespace gui {

class Loader : NonCopyable
{
public:
    Loader(const string& resource_name);
    P<Widget> create(const string& root_id, P<Widget> root=nullptr, bool auto_reload=false);

    static P<Widget> load(const string& resource_name, const string& root_id, P<Widget> root=nullptr, bool auto_reload=false);

private:
    class SubLoader
    {
    public:
        SubLoader(Loader& loader, const string& resource_name);
        
        P<Widget> createWidget(P<Widget> parent, KeyValueTreeNode& node, std::map<string, string>& parameters);
        void loadWidgetFromTree(P<Widget> widget, KeyValueTreeNode& node, std::map<string, string>& parameters);
        string getType(KeyValueTreeNode& node, std::map<string, string>& parameters);
        KeyValueTreeNode* findRef(const string& reference_name);

        Loader& loader;
        KeyValueTreePtr tree;
    };

    string primary_resource_name;
    std::map<string, SubLoader> subs;
};

}//namespace gui
}//namespace sp

#endif//SP2_GRAPHICS_GUI_LOADER_H
