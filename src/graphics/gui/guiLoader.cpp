#include <sp2/graphics/gui/guiLoader.h>
#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/io/keyValueTreeLoader.h>
#include <sp2/logging.h>

namespace sp {
namespace gui {

P<Widget> Loader::load(string resource_name, string root_id, P<Widget> root_widget, bool auto_reload)
{
    Loader loader;
    loader.tree = io::KeyValueTreeLoader::load(resource_name);
    KeyValueTreeNode* root = loader.tree->findId(root_id);

    if (root)
    {
        P<Widget> result = loader.createWidget(root_widget, *root);
        if (auto_reload)
        {
#ifdef DEBUG
            root_widget->setupAutoReload(result, resource_name, root_id);
#else
            LOG(Warning, "Cannot use 'auto reload' on sp::gui::Loader in release builds");
#endif
        }
        return result;
    }
    return nullptr;
}

P<Widget> Loader::createWidget(P<Widget> parent, KeyValueTreeNode& node)
{
    string type = getType(node);
    WidgetClassRegistry* reg;
    for(reg = WidgetClassRegistry::first; reg != nullptr; reg = reg->next)
    {
        if (type == reg->name)
            break;
    }
    if (reg)
    {
        P<Widget> widget = reg->creation_function(parent);
        widget->setID(node.id);
        loadWidgetFromTree(widget, node);
        return widget;
    }else{
        LOG(Error, "Failed to find widget type:", type);
    }
    return nullptr;
}

void Loader::loadWidgetFromTree(P<Widget> widget, KeyValueTreeNode& node)
{
    if (node.items.find("@ref") != node.items.end())
    {
        KeyValueTreeNode* ref = tree->findId("@" + node.items["@ref"]);
        if (ref)
            loadWidgetFromTree(widget, *ref);
        else
            LOG(Warning, "Could not find @ref", node.items["@ref"]);
    }
    for(auto it : node.items)
    {
        if (it.first != "@ref" && it.first != "type")
        {
            widget->setAttribute(it.first, it.second);
        }
    }
    
    for(KeyValueTreeNode& child_node : node.child_nodes)
    {
        createWidget(widget, child_node);
    }
}

string Loader::getType(KeyValueTreeNode& node)
{
    if (node.items.find("type") != node.items.end())
        return node.items["type"];
    if (node.items.find("@ref") != node.items.end())
    {
        KeyValueTreeNode* ref = tree->findId("@" + node.items["@ref"]);
        if (ref)
            return getType(*ref);
        else
            LOG(Warning, "Could not find @ref", node.items["@ref"]);
    }
    return "";
}

};//!namespace gui
};//!namespace sp
