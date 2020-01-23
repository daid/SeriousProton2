#include <sp2/io/serialization/dataset.h>
#include <sp2/io/serialization/serializer.h>

namespace sp {
namespace io {
namespace serialization {

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

void DataSet::set(const char* key, float value)
{
    addAsRawData(key, DataType::Float, &value, sizeof(value));
}

void DataSet::set(const char* key, double value)
{
    addAsRawData(key, DataType::Double, &value, sizeof(value));
}

void DataSet::set(const char* key, const Vector2i& value)
{
    addAsRawData(key, DataType::Vector2i, &value, sizeof(value));
}

void DataSet::set(const char* key, const Vector2f& value)
{
    addAsRawData(key, DataType::Vector2f, &value, sizeof(value));
}

void DataSet::set(const char* key, const Vector2d& value)
{
    addAsRawData(key, DataType::Vector2d, &value, sizeof(value));
}

void DataSet::set(const char* key, const Vector3i& value)
{
    addAsRawData(key, DataType::Vector3i, &value, sizeof(value));
}

void DataSet::set(const char* key, const Vector3f& value)
{
    addAsRawData(key, DataType::Vector3f, &value, sizeof(value));
}

void DataSet::set(const char* key, const Vector3d& value)
{
    addAsRawData(key, DataType::Vector3d, &value, sizeof(value));
}

void DataSet::set(const char* key, const string& value)
{
    size_t index = data.size();
    values[serializer.getStringIndex(key)] = {DataType::String, index};
    size_t string_length = value.length();
    while(string_length > 127)
    {
        data.push_back(0x80 | (string_length & 0x7f));
        string_length >>= 7;
    }
    data.push_back(string_length);
    index = data.size();
    data.resize(data.size() + value.length());
    memcpy(&data[index], value.data(), value.length());
}

template<> int DataSet::get(const char* key) const
{
    int idx = serializer.getStringIndex(key);
    if (idx < 0)
        return 0;
    auto it = values.find(idx);
    if (it == values.end() || it->second.first != DataType::Integer)
        return 0;
    return it->second.second;
}

template<> float DataSet::get(const char* key) const
{
    float result = 0.0;
    getAsRawData(key, DataType::Float, &result, sizeof(result));
    return result;
}
template<> double DataSet::get(const char* key) const
{
    double result = 0.0;
    getAsRawData(key, DataType::Double, &result, sizeof(result));
    return result;
}
template<> Vector2i DataSet::get(const char* key) const
{
    Vector2i result;
    getAsRawData(key, DataType::Vector2i, &result, sizeof(result));
    return result;
}
template<> Vector2f DataSet::get(const char* key) const
{
    Vector2f result;
    getAsRawData(key, DataType::Vector2f, &result, sizeof(result));
    return result;
}
template<> Vector2d DataSet::get(const char* key) const
{
    Vector2d result;
    getAsRawData(key, DataType::Vector2d, &result, sizeof(result));
    return result;
}
template<> Vector3i DataSet::get(const char* key) const
{
    Vector3i result;
    getAsRawData(key, DataType::Vector3i, &result, sizeof(result));
    return result;
}
template<> Vector3f DataSet::get(const char* key) const
{
    Vector3f result;
    getAsRawData(key, DataType::Vector3f, &result, sizeof(result));
    return result;
}
template<> Vector3d DataSet::get(const char* key) const
{
    Vector3d result;
    getAsRawData(key, DataType::Vector3d, &result, sizeof(result));
    return result;
}
template<> string DataSet::get(const char* key) const
{
    int idx = serializer.getStringIndex(key);
    if (idx < 0)
        return "";
    auto it = values.find(idx);
    if (it == values.end() || it->second.first != DataType::String)
        return "";
    size_t index = it->second.second;
    size_t length = 0;
    size_t shift = 0;
    while(data[index] & 0x80)
    {
        length = length | ((data[index] & 0x7f) << shift);
        shift += 7;
        index += 1;
    }
    length = length | (data[index] << shift);
    index += 1;
    return string(reinterpret_cast<const char*>(&data[index]), length);
}

void DataSet::addAsRawData(const char* key, DataType type, const void* ptr, size_t size)
{
    size_t index = data.size();
    values[serializer.getStringIndex(key)] = {type, index};
    data.resize(data.size() + size);
    memcpy(&data[index], ptr, size);
}

bool DataSet::getAsRawData(const char* key, DataType type, void* ptr, size_t size) const
{
    int idx = serializer.getStringIndex(key);
    if (idx < 0)
        return false;
    auto it = values.find(idx);
    if (it == values.end() || it->second.first != type)
        return false;
    memcpy(ptr, &data[it->second.second], size);
    return true;
}

} // namespace
} // namespace
} // namespace
