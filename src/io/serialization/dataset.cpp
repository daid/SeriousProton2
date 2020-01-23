#include <sp2/io/serialization/dataset.h>
#include <sp2/io/serialization/serializer.h>

namespace sp::io::serialization {

DataSet::DataSet(Serializer& serializer)
: serializer(serializer)
{
}

DataSet::~DataSet()
{
}

void DataSet::set(const char* key, int value)
{
    values[serializer.getStringIndex(key)] = {DataType::Integer, value};
}

template<> int DataSet::get(const char* key) const
{
    int idx = serializer.getStringIndex(key);
    if (idx < 0)
        return 0;
    auto it = values.find(idx);
    if (it == values.end())
        return 0;
    return it->second.second;
}

} // namespace
