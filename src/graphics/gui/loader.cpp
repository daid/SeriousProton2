#include <sp2/graphics/gui/loader.h>
#include <sp2/graphics/gui/scene.h>
#include <sp2/graphics/gui/widget/root.h>
#include <sp2/io/keyValueTreeLoader.h>
#include <sp2/logging.h>
#include <sp2/assert.h>

namespace sp {
namespace gui {

Loader::Loader(const sp::string& resource_name)
: primary_resource_name(resource_name)
{
    subs.emplace(resource_name, SubLoader(*this, resource_name));
}

P<Widget> Loader::create(const string& root_id, P<Widget> root_widget, bool auto_reload)
{
    SubLoader& sub = subs.find(primary_resource_name)->second;
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
            root_widget->setupAutoReload(result, primary_resource_name, root_id);
#else
            LOG(Warning, "Cannot use 'auto reload' on sp::gui::Loader in release builds");
#endif
        }
        return result;
    }
    return nullptr;
}

P<Widget> Loader::load(const string& resource_name, const string& root_id, P<Widget> root_widget, bool auto_reload)
{
    return Loader(resource_name).create(root_id, root_widget, auto_reload);
}

Loader::SubLoader::SubLoader(Loader& loader, const string& resource_name)
: loader(loader)
{
    tree = io::KeyValueTreeLoader::loadResource(resource_name);
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
        widget->postLoading();
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
        string args = node.items["@ref"].format(parameters);
        std::map<string, string> new_parameters = parameters;

        int start_of_value = args.rfind("=");
        while(start_of_value > -1)
        {
            string value = args.substr(start_of_value + 1);
            args = args.substr(0, start_of_value);
            int start_of_key = args.rfind(" ");
            string key = args.substr(start_of_key + 1);
            args = args.substr(0, start_of_key);
            new_parameters[key] = value;

            start_of_value = args.rfind("=");
        }
        string reference_name = args;

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

}//namespace gui
}//namespace sp
