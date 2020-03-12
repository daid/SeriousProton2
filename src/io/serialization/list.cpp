#include <sp2/io/serialization/list.h>
#include <sp2/io/serialization/dataset.h>
#include <sp2/io/serialization/serializer.h>

namespace sp {
namespace io {
namespace serialization {

List::List(DataSet* dataset, int key_index)
: dataset(dataset), key_index(key_index)
{
}

List::List(List&& other)
: ids(std::move(other.ids)), dataset(other.dataset), key_index(other.key_index)
{
    other.dataset = nullptr;
}

List::~List()
{
    if (dataset)
    {
        size_t index = dataset->data.size();
        dataset->values[key_index] = {DataSet::DataType::List, index};
        dataset->pushUInt(ids.size());
        for(auto id : ids)
            dataset->pushUInt(id);
    }
}

DataSet List::next()
{
    DataSet data(*dataset->serializer, dataset->serializer->next_dataset_id);
    ids.push_back(dataset->serializer->next_dataset_id);
    dataset->serializer->next_dataset_id += 1;
    return data;
}

} // namespace
} // namespace
} // namespace
