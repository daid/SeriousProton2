#ifndef SP2_GRAPHICS_GUI_LOADER_H
#define SP2_GRAPHICS_GUI_LOADER_H

#include <sp2/string.h>
#include <sp2/pointer.h>
#include <sp2/keyValueTree.h>

namespace sp {
namespace gui {

class Widget;

class Loader : NonCopyable
{
public:
    static P<Widget> load(string resource_name, string root_id, P<Widget> root=nullptr, bool auto_reload=false);

private:
    class SubLoader
    {
    public:
        SubLoader(Loader& loader, string resource_name);
        
        P<Widget> createWidget(P<Widget> parent, KeyValueTreeNode& node, std::map<string, string>& parameters);
        void loadWidgetFromTree(P<Widget> widget, KeyValueTreeNode& node, std::map<string, string>& parameters);
        string getType(KeyValueTreeNode& node, std::map<string, string>& parameters);
        KeyValueTreeNode* findRef(const string& reference_name);

        Loader& loader;
        KeyValueTreePtr tree;
    };
    
    std::map<string, SubLoader> subs;
};

};//namespace gui
};//namespace sp

#endif//SP2_GRAPHICS_GUI_LOADER_H
