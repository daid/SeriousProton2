#include <sp2/io/serialization/dataset.h>
#include <sp2/io/serialization/serializer.h>

namespace sp {
namespace io {
namespace serialization {

DataSet::DataSet(Serializer& serializer, int id)
: serializer(serializer), id(id)
{
}

DataSet::~DataSet()
{
    if (serializer.mode == Serializer::Mode::Write)
    {
        serializer.requestWriteFrom(*this);
    }
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
    pushUInt(value.length());
    index = data.size();
    data.resize(data.size() + value.length());
    memcpy(&data[index], value.data(), value.length());
}

void DataSet::set(const char* key, P<AutoPointerObject> obj)
{
    if (!obj)
        return;

    auto it = serializer.object_to_id.find(*obj);
    if (it != serializer.object_to_id.end())
    {
        values[serializer.getStringIndex(key)] = {DataType::AutoPointerObject, it->second};
        return;
    }

    auto t = std::type_index(typeid(**obj));
    auto type_to_name = serializer.type_to_name_mapping.find(t);
    sp2assert(type_to_name != serializer.type_to_name_mapping.end(), "Missing class registration for serialization");

    serializer.object_to_id[*obj] = serializer.next_dataset_id;
    DataSet dataset(serializer, serializer.next_dataset_id);
    serializer.next_dataset_id += 1;
    dataset.set("class", serializer.getStringIndex(type_to_name->second));
    serializer.type_to_save_function_mapping[t](*obj, dataset);
    values[serializer.getStringIndex(key)] = {DataType::AutoPointerObject, dataset.getId()};
}

void DataSet::createList(const char* key, std::function<void(List&)> callback)
{
    List list(serializer);
    callback(list);

    size_t index = data.size();
    values[serializer.getStringIndex(key)] = {DataType::List, index};
    pushUInt(list.ids.size());
    for(auto id : list.ids)
        pushUInt(id);
}

bool DataSet::has(const char* key) const
{
    int idx = serializer.getStringIndex(key);
    if (idx < 0)
        return false;
    return values.find(idx) != values.end();
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
    int index = it->second.second;
    int length = pullUInt(index);
    if (length < 0 || index < 0 || size_t(index + length) > data.size())
        return "";
    return string(reinterpret_cast<const char*>(&data[index]), length);
}

P<AutoPointerObject> DataSet::getObject(const char* key) const
{
    int idx = serializer.getStringIndex(key);
    if (idx < 0)
        return nullptr;
    auto it = values.find(idx);
    if (it == values.end() || it->second.first != DataType::AutoPointerObject)
        return nullptr;
    return serializer.getStoredObject(it->second.second);
}

void DataSet::getList(const char* key, std::function<void(const DataSet&)> callback) const
{
    int idx = serializer.getStringIndex(key);
    if (idx < 0)
        return;
    auto it = values.find(idx);
    if (it == values.end() || it->second.first != DataType::List)
        return;
    int index = it->second.second;
    int count = pullUInt(index);
    while(count > 0)
    {
        int id = pullUInt(index);
        DataSet data(serializer, id);
        fseeko(serializer.f, serializer.dataset_file_position[id], SEEK_SET);
        data.readFromFile(serializer.f);
        callback(data);
        count -= 1;
    }
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
    if (it->second.second < 0 || it->second.second + size >= data.size())
        return false;
    memcpy(ptr, &data[it->second.second], size);
    return true;
}

void DataSet::pushUInt(int n)
{
    while(n > 127)
    {
        data.push_back(0x80 | (n & 0x7f));
        n >>= 7;
    }
    data.push_back(n);
}

int DataSet::pullUInt(int& index) const
{
    if (index < 0)
        return 0;
    int result = 0;
    size_t shift = 0;
    while(data[index] & 0x80)
    {
        result = result | ((data[index] & 0x7f) << shift);
        shift += 7;
        index += 1;
        if (size_t(index) >= data.size())
            return 0;
    }
    result = result | (data[index] << shift);
    index += 1;
    return result;
}

void DataSet::writeToFile(FILE* f)
{
    writeUInt(values.size(), f);
    for(auto it : values)
    {
        writeUInt(it.first, f);
        if (it.second.second < 0)
            writeUInt(static_cast<int>(it.second.first) | 0x40, f);
        else
            writeUInt(static_cast<int>(it.second.first), f);
        writeUInt(std::abs(it.second.second), f);
    }

    writeUInt(data.size(), f);
    fwrite(data.data(), data.size(), 1, f);
}

void DataSet::readFromFile(FILE* f)
{
    unsigned int value_count = readUInt(f);
    for(unsigned int n=0; n<value_count; n++)
    {
        int string_index = readUInt(f);
        int type = readUInt(f);
        int value = readUInt(f);
        if (type & 0x40)
        {
            type &= 0x3f;
            value =-value;
        }
        values[string_index] = {static_cast<DataType>(type), value};
    }

    data.resize(readUInt(f));
    if (fread(data.data(), data.size(), 1, f) != 1)
        data.clear();
}

void DataSet::writeUInt(unsigned int v, FILE* f)
{
    while(v > 127)
    {
        fputc((v & 0x7f) | 0x80, f);
        v >>= 7;
    }
    fputc(v, f);
}

unsigned int DataSet::readUInt(FILE* f)
{
    unsigned int result = 0;
    size_t shift = 0;
    int c;
    for(c = fgetc(f); c & 0x80; c = fgetc(f))
    {
        if (feof(f))
            return 0;
        result |= (c & 0x7f) << shift;
        shift += 7;
    }
    result |= c << shift;
    return result;
}

} // namespace
} // namespace
} // namespace
