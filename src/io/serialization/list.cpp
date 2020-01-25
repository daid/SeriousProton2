#include <sp2/io/serialization/list.h>
#include <sp2/io/serialization/dataset.h>
#include <sp2/io/serialization/serializer.h>

namespace sp {
namespace io {
namespace serialization {

List::List(Serializer& serializer)
: serializer(serializer)
{
}

List::~List()
{
}

void List::next(std::function<void(DataSet&)> callback)
{
    DataSet data(serializer, serializer.next_dataset_id);
    ids.push_back(serializer.next_dataset_id);
    serializer.next_dataset_id += 1;
    callback(data);
}

} // namespace
} // namespace
} // namespace
