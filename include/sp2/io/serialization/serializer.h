#ifndef SP2_IO_SERIALIZATION_SERIALIZER_H
#define SP2_IO_SERIALIZATION_SERIALIZER_H

#include <sp2/string.h>
#include <sp2/nonCopyable.h>
#include <sp2/math/vector.h>
#include <sp2/pointer.h>
#include <sp2/io/serialization/dataset.h>
#include <sp2/assert.h>
#include <sp2/logging.h>
#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#include <functional>


namespace sp {
namespace io {
namespace serialization {

class Serializer : public DataSet
{
public:
    using createFunctionType = AutoPointerObject*(*)(const DataSet&);

    enum class Mode{Read, Write};

    explicit Serializer(const string& filename, Mode mode);
    ~Serializer();

    template<typename T, typename std::enable_if<std::is_same<decltype(&T::create), createFunctionType>::value, int>::type = 0>
    void registerClass(const sp::string& name)
    {
        auto t = std::type_index(typeid(T));
        type_to_name_mapping[t] = name;
        type_to_save_function_mapping[t] = [](AutoPointerObject* obj, DataSet& data)
        {
            static_cast<T*>(obj)->save(data);
        };
        name_to_create_function_mapping[name] = &T::create;
    }

protected:
    int getStringIndex(const sp::string& s);
    void requestWriteFrom(DataSet& dataset);
    P<AutoPointerObject> getStoredObject(int id);

private:
    void finishFile();
    void initialRead();

    Mode mode;
    FILE* f = nullptr;
    std::unordered_map<std::type_index, sp::string> type_to_name_mapping;
    std::unordered_map<std::type_index, std::function<void(AutoPointerObject*, DataSet&)>> type_to_save_function_mapping;
    std::unordered_map<string, createFunctionType> name_to_create_function_mapping;

    std::unordered_map<int, off_t> dataset_file_position;
    std::unordered_map<string, int> string_table;

    std::unordered_map<AutoPointerObject*, int> object_to_id;
    std::unordered_map<int, AutoPointerObject*> id_to_object;
    int next_dataset_id = 1;

    friend class DataSet;
    friend class List;
};

}//namespace
}//namespace
}//namespace

#endif//SP2_IO_SERIALIZATION_SERIALIZER_H
