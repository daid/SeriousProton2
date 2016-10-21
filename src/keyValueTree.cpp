#include <sp2/keyValueTree.h>

namespace sp {

KeyValueTreeNode* KeyValueTree::findId(string id)
{
    for(KeyValueTreeNode& node : root_nodes)
    {
        KeyValueTreeNode* result = findId(node, id);
        if (result)
            return result;
    }
    return nullptr;
}

std::map<string, std::map<string, string>> KeyValueTree::getFlattenNodesByIds()
{
    std::map<string, std::map<string, string>> results;
    for(const KeyValueTreeNode& node : root_nodes)
    {
        buildFlattenNodesByIds(results, node, {});
    }
    return results;
}

KeyValueTreeNode* KeyValueTree::findId(KeyValueTreeNode& node, const string& id)
{
    if (node.id == id)
        return &node;
    for(KeyValueTreeNode& node : node.child_nodes)
    {
        KeyValueTreeNode* result = findId(node, id);
        if (result)
            return result;
    }
    return nullptr;
}

void KeyValueTree::buildFlattenNodesByIds(std::map<string, std::map<string, string>>& results, const KeyValueTreeNode& node, std::map<string, string> key_values)
{
    for(auto it : node.items)
        key_values[it.first] = it.second;
    if (node.id != "")
        results[node.id] = key_values;
    for(const KeyValueTreeNode& child : node.child_nodes)
    {
        buildFlattenNodesByIds(results, child, key_values);
    }
}

};//!namespace sp
