#ifndef SP2_KEY_VALUE_TREE_H
#define SP2_KEY_VALUE_TREE_H

#include <sp2/string.h>
#include <sp2/nonCopyable.h>
#include <map>
#include <memory>

namespace sp {

class KeyValueTreeNode
{
public:
    string id;
    std::map<string, string> items;
    
    std::vector<KeyValueTreeNode> child_nodes;
};

class KeyValueTree : public NonCopyable
{
public:
    std::vector<KeyValueTreeNode> root_nodes;
    
    KeyValueTreeNode* findId(string id);
    std::map<string, std::map<string, string>> getFlattenNodesByIds();
private:
    KeyValueTreeNode* findId(KeyValueTreeNode& node, const string& id);
    void buildFlattenNodesByIds(std::map<string, std::map<string, string>>& results, const KeyValueTreeNode& node, std::map<string, string> key_values);
};
typedef std::shared_ptr<KeyValueTree> KeyValueTreePtr;

};//!namespace sp

#endif//SP2_KEY_VALUE_TREE_H
