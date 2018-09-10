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
    loader.subs.emplace(resource_name, SubLoader(loader, resource_name));
    SubLoader& sub = loader.subs.find(resource_name)->second;
    if (!sub.tree)
        return nullptr;
    KeyValueTreeNode* root = sub.tree->findId(root_id);
    if (!root_widget)
    {
        sp2assert(Scene::default_gui_scene, "Need to create a <sp::gui::Scene> before Widgets can be created");
        root_widget = Scene::default_gui_scene->getRootWidget();
    }

    if (root)
    {
        std::map<string, string> parameters;
        P<Widget> result = sub.createWidget(root_widget, *root, parameters);
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

Loader::SubLoader::SubLoader(Loader& loader, string resource_name)
: loader(loader)
{
    tree = io::KeyValueTreeLoader::load(resource_name);
}

P<Widget> Loader::SubLoader::createWidget(P<Widget> parent, KeyValueTreeNode& node, std::map<string, string>& parameters)
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

void Loader::SubLoader::loadWidgetFromTree(P<Widget> widget, KeyValueTreeNode& node, std::map<string, string>& parameters)
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
        
        KeyValueTreeNode* ref = findRef(reference_name);
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

string Loader::SubLoader::getType(KeyValueTreeNode& node, std::map<string, string>& parameters)
{
    if (node.items.find("type") != node.items.end())
        return node.items["type"];
    if (node.items.find("@ref") != node.items.end())
    {
        auto values = node.items["@ref"].format(parameters).split();
        string reference_name = values.front();
        KeyValueTreeNode* ref = findRef(reference_name);
        if (ref)
            return getType(*ref, parameters);
        else
            LOG(Warning, "Could not find @ref", reference_name);
    }
    return "";
}

KeyValueTreeNode* Loader::SubLoader::findRef(const string& reference_name)
{
    KeyValueTreeNode* ref = nullptr;
    int ref_split = reference_name.find("#");
    if (ref_split > -1)
    {
        string resource_name = reference_name.substr(0, ref_split);
        auto it = loader.subs.find(resource_name);
        if (it != loader.subs.end())
            return it->second.findRef(reference_name.substr(ref_split + 1));
        loader.subs.emplace(resource_name, SubLoader(loader, resource_name));
        return loader.subs.find(resource_name)->second.findRef(reference_name.substr(ref_split + 1));
    }
    else if (tree)
    {
        ref = tree->findId("@" + reference_name);
    }
    return ref;
}

};//namespace gui
};//namespace sp
