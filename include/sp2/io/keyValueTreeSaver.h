#ifndef SP2_IO_KEYVALUETREE_SAVER_H
#define SP2_IO_KEYVALUETREE_SAVER_H

#include <sp2/keyValueTree.h>

namespace sp {
namespace io {

class KeyValueTreeSaver
{
public:
    static void save(const string& filename, const KeyValueTree& tree);

private:
    FILE* f;
    
    KeyValueTreeSaver(const string& filename);
    ~KeyValueTreeSaver();
    void saveNode(const KeyValueTreeNode& node, int indent);
};

}//namespace io
}//namespace sp

#endif//SP2_IO_KEYVALUETREE_LOADER_H
