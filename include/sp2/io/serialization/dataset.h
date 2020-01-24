#ifndef SP2_IO_SERIALIZATION_DATASET_H
#define SP2_IO_SERIALIZATION_DATASET_H

#include <sp2/string.h>
#include <sp2/nonCopyable.h>
#include <sp2/math/vector.h>
#include <sp2/pointer.h>
#include <unordered_map>

namespace sp {
namespace io {
namespace serialization {

class Serializer;
class DataSet : NonCopyable
{
public:
    void set(const char* key, int value);
    template<typename T, typename std::enable_if<std::is_enum<T>::value, int>::type = 0>
    void set(const char* key, T value) { set(key, static_cast<int>(value)); }
    void set(const char* key, float value);
    void set(const char* key, double value);
    void set(const char* key, const Vector2i& value);
    void set(const char* key, const Vector2f& value);
    void set(const char* key, const Vector2d& value);
    void set(const char* key, const Vector3i& value);
    void set(const char* key, const Vector3f& value);
    void set(const char* key, const Vector3d& value);
    void set(const char* key, const string& value);
    void set(const char* key, P<AutoPointerObject> obj);

    template<typename T, typename std::enable_if<!std::is_enum<T>::value, int>::type = 0>
    T get(const char* key) const = delete;
    template<typename T, typename std::enable_if<std::is_enum<T>::value, int>::type = 0>
    T get(const char* key) const;

    P<AutoPointerObject> getObject(const char* key) const;
protected:
    DataSet(Serializer& serializer, int id);
    ~DataSet();

    void writeToFile(FILE* f);
    void readFromFile(FILE* f);

    int getId() { return id; }

    static void writeUInt(unsigned int v, FILE* f);
    static unsigned int readUInt(FILE* f);
private:
    enum class DataType
    {
        Integer,
        Float,
        Double,
        Vector2i,
        Vector2f,
        Vector2d,
        Vector3i,
        Vector3f,
        Vector3d,
        String,
        AutoPointerObject,
    };

    void addAsRawData(const char* key, DataType type, const void* ptr, size_t size);
    bool getAsRawData(const char* key, DataType type, void* ptr, size_t size) const;

    std::unordered_map<int, std::pair<DataType, int>> values;
    std::vector<uint8_t> data;

    Serializer& serializer;
    int id;

    friend class Serializer;
};
template<> int DataSet::get(const char* key) const;
template<typename T, typename std::enable_if<std::is_enum<T>::value, int>::type>
T DataSet::get(const char* key) const
{
    return static_cast<T>(get<int>(key));
}
template<> float DataSet::get(const char* key) const;
template<> double DataSet::get(const char* key) const;
template<> Vector2i DataSet::get(const char* key) const;
template<> Vector2f DataSet::get(const char* key) const;
template<> Vector2d DataSet::get(const char* key) const;
template<> Vector3i DataSet::get(const char* key) const;
template<> Vector3f DataSet::get(const char* key) const;
template<> Vector3d DataSet::get(const char* key) const;
template<> string DataSet::get(const char* key) const;

}//namespace
}//namespace
}//namespace

#endif//SP2_IO_SERIALIZATION_DATASET_H
