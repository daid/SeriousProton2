#ifndef SP2_IO_SERIALIZATION_LIST_H
#define SP2_IO_SERIALIZATION_LIST_H

#include <sp2/nonCopyable.h>
#include <vector>
#include <functional>


namespace sp {
namespace io {
namespace serialization {

class Serializer;
class DataSet;
class List : NonCopyable
{
public:
    DataSet next();

    List(List&& other);
    ~List();
protected:
    List(DataSet* dataset, int key_index);

    std::vector<int> ids;
private:
    DataSet* dataset;
    int key_index;

    friend class DataSet;
};

}//namespace
}//namespace
}//namespace

#endif//SP2_IO_SERIALIZATION_LIST_H
