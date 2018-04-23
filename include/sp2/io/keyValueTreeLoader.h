#ifndef SP2_IO_KEYVALUETREE_LOADER_H
#define SP2_IO_KEYVALUETREE_LOADER_H

#include <sp2/keyValueTree.h>
#include <sp2/io/resourceProvider.h>

namespace sp {
namespace io {

class KeyValueTreeLoader
{
public:
    static KeyValueTreePtr load(string resource_name);

private:
    KeyValueTreePtr result;
    ResourceStreamPtr stream;
    
    KeyValueTreeLoader(string resource_name);
    void parseNode(KeyValueTreeNode* node);
};

};//namespace io
};//namespace sp

#endif//SP2_IO_KEYVALUETREE_LOADER_H
