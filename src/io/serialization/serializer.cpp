#include <sp2/io/serialization/serializer.h>

namespace sp {
namespace io {
namespace serialization {

Serializer::Serializer(const string& filename, Mode mode)
: DataSet(*this), mode(mode)
{
}

Serializer::~Serializer()
{
}

int Serializer::getStringIndex(const string& s)
{
    auto it = string_table.find(s);
    if (it != string_table.end())
        return it->second;
    if (mode == Mode::Read)
        return -1;
    string_table[s] = string_table.size();
    return string_table.size() - 1;
}

} // namespace
} // namespace
} // namespace
