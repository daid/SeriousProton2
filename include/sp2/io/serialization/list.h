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
    void next(std::function<void(DataSet&)> callback);

protected:
    List(Serializer& serializer);
    ~List();

    std::vector<int> ids;
private:
    Serializer& serializer;

    friend class DataSet;
};

}//namespace
}//namespace
}//namespace

#endif//SP2_IO_SERIALIZATION_LIST_H
