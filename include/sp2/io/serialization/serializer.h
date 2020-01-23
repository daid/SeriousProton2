#ifndef SP2_IO_SERIALIZATION_SERIALIZER_H
#define SP2_IO_SERIALIZATION_SERIALIZER_H

#include <sp2/string.h>
#include <sp2/nonCopyable.h>
#include <sp2/math/vector.h>
#include <sp2/pointer.h>
#include <unordered_map>
#include <typeinfo>
#include <typeindex>

#include <sp2/io/serialization/dataset.h>

namespace sp {
namespace io {
namespace serialization {

class Serializer : public DataSet
{
public:
    using createFunctionType = std::add_pointer<sp::AutoPointerObject*(const DataSet&)>;

    enum class Mode{Read, Write};

    explicit Serializer(const string& filename, Mode mode);
    ~Serializer();

    template<typename T, typename std::enable_if<std::is_same<std::add_pointer<decltype(T::create)>, createFunctionType>::value, int>::type = 0>
    void registerClass(const sp::string& name)
    {
        type_to_name_mapping[std::type_index(typeid(T))] = name;
        name_to_create_function_mapping[name] = &T::create;
    }

protected:
    int getStringIndex(const sp::string& s);
    void requestWriteFrom(DataSet& dataset);

private:
    void finishFile();
    void initialRead();

    Mode mode;
    FILE* f = nullptr;
    std::unordered_map<std::type_index, sp::string> type_to_name_mapping;
    std::unordered_map<string, createFunctionType> name_to_create_function_mapping;

    std::unordered_map<int, off_t> dataset_file_position;
    std::unordered_map<string, int> string_table;

    friend class DataSet;
};

}//namespace
}//namespace
}//namespace

#endif//SP2_IO_SERIALIZATION_SERIALIZER_H
