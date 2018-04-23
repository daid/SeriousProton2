#include <sp2/graphics/gui/loader.h>
#include <sp2/graphics/gui/scene.h>
#include <sp2/graphics/gui/widget/root.h>
#include <sp2/io/keyValueTreeLoader.h>
#include <sp2/logging.h>
#include <sp2/assert.h>

namespace sp {
namespace gui {

P<Widget> Loader::load(string resource_name, string root_id, P<Widget> root_widget, bool auto_reload)
{
    Loader loader;
    loader.tree = io::KeyValueTreeLoader::load(resource_name);
    if (!loader.tree)
        return nullptr;
    KeyValueTreeNode* root = loader.tree->findId(root_id);
    if (!root_widget)
    {
        sp2assert(Scene::default_gui_scene, "Need to create a <sp::gui::Scene> before Widgets can be created");
        root_widget = Scene::default_gui_scene->getRootWidget();
    }

    if (root)
    {
        std::map<string, string> parameters;
        P<Widget> result = loader.createWidget(root_widget, *root, parameters);
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

P<Widget> Loader::createWidget(P<Widget> parent, KeyValueTreeNode& node, std::map<string, string>& parameters)
{
    string type = getType(node, parameters);
    WidgetClassRegistry* reg;
    for(reg = WidgetClassRegistry::first; reg != nullptr; reg = reg->next)
    {
        if (type == reg->name)
            break;
    }
    if (reg)
    {
        P<Widget> widget = reg->creation_function(parent);
        widget->setID(node.id.format(parameters));
        loadWidgetFromTree(widget, node, parameters);
        return widget;
    }else{
        LOG(Error, "Failed to find widget type:", type);
    }
    return nullptr;
}

void Loader::loadWidgetFromTree(P<Widget> widget, KeyValueTreeNode& node, std::map<string, string>& parameters)
{
    if (node.items.find("@ref") != node.items.end())
    {
        auto values = node.items["@ref"].format(parameters).split();
        string reference_name = values.front();
        values.erase(values.begin());
        
        std::map<string, string> new_parameters = parameters;
        for(auto parameter : values)
        {
            auto key_value = parameter.split("=", 1);
            if (key_value.size() > 1)
                new_parameters[key_value[0]] = key_value[1];
        }
        
        KeyValueTreeNode* ref = tree->findId("@" + reference_name);
        if (ref)
            loadWidgetFromTree(widget, *ref, new_parameters);
        else
            LOG(Warning, "Could not find @ref", reference_name);
    }
    for(auto it : node.items)
    {
        if (it.first != "@ref" && it.first != "type")
        {
            widget->setAttribute(it.first, it.second.format(parameters));
        }
    }
    
    for(KeyValueTreeNode& child_node : node.child_nodes)
    {
        createWidget(widget, child_node, parameters);
    }
}

string Loader::getType(KeyValueTreeNode& node, std::map<string, string>& parameters)
{
    if (node.items.find("type") != node.items.end())
        return node.items["type"];
    if (node.items.find("@ref") != node.items.end())
    {
        auto values = node.items["@ref"].format(parameters).split();
        string reference_name = values.front();
        KeyValueTreeNode* ref = tree->findId("@" + reference_name);
        if (ref)
            return getType(*ref, parameters);
        else
            LOG(Warning, "Could not find @ref", reference_name);
    }
    return "";
}

};//namespace gui
};//namespace sp
