#include <sp2/keyValueTree.h>

namespace sp {

const KeyValueTreeNode* KeyValueTreeNode::findId(const string& id) const
{
    if (id == this->id)
        return this;
    for(const KeyValueTreeNode& node : child_nodes)
    {
        const KeyValueTreeNode* result = node.findId(id);
        if (result)
            return result;
    }
    return nullptr;
}

KeyValueTreeNode* KeyValueTreeNode::findId(const string& id)
{
    if (id == this->id)
        return this;
    for(KeyValueTreeNode& node : child_nodes)
    {
        KeyValueTreeNode* result = node.findId(id);
        if (result)
            return result;
    }
    return nullptr;
}

KeyValueTreeNode* KeyValueTree::findId(const string& id)
{
    for(KeyValueTreeNode& node : root_nodes)
    {
        KeyValueTreeNode* result = node.findId(id);
        if (result)
            return result;
    }
    return nullptr;
}

static void searchAndAdd(std::vector<KeyValueTreeNode>& nodes, std::vector<KeyValueTreeNode*>& result, const string& id) {
    for(auto& node : nodes) {
        if (node.id == id)
            result.push_back(&node);
        searchAndAdd(node.child_nodes, result, id);
    }
}

std::vector<KeyValueTreeNode*> KeyValueTree::findAllId(const string& id)
{
    std::vector<KeyValueTreeNode*> result;
    searchAndAdd(root_nodes, result, id);
    return result;
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

}//namespace sp
