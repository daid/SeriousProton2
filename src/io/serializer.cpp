#include <sp2/io/serializer.h>
#include <sp2/logging.h>
#include <sp2/assert.h>

#include <string.h>
#include <algorithm>

namespace sp {
namespace io {

Serializer::Serializer(string filename)
: handler(filename)
{
}

bool Serializer::read(string name, ISerializable& object)
{
    return handler.read(name, object);
}

void Serializer::write(string name, ISerializable& object)
{
    handler.write(name, object);
}

Serializer::Handler::Handler(string filename)
: filename(filename)
{
}

Serializer::Handler::~Handler()
{
    if (mode == Mode::Write)
    {
        FILE* f = fopen(filename.c_str(), "wb");
        if (!f)
            return;
        fwrite("SP2:data", 8, 1, f);
        std::vector<uint8_t> header_data;
        current_data_block = &header_data;
        addUnsignedInt(1);
        addUnsignedInt(string_table.size());
        addUnsignedInt(main_objects.size());
        addUnsignedInt(data_blocks.size());
        fwrite(header_data.data(), header_data.size(), 1, f);
        for(string s : string_table)
        {
            header_data.clear();
            addUnsignedInt(s.size());
            fwrite(header_data.data(), header_data.size(), 1, f);
            fwrite(s.c_str(), s.size(), 1, f);
        }
        header_data.clear();
        for(auto it : main_objects)
        {
            addUnsignedInt(it.first);
            addUnsignedInt(it.second);
        }
        fwrite(header_data.data(), header_data.size(), 1, f);
        for(auto& block : data_blocks)
        {
            header_data.clear();
            addUnsignedInt(block.size());
            fwrite(header_data.data(), header_data.size(), 1, f);
            fwrite(block.data(), block.size(), 1, f);
        }
        fclose(f);
    }
}

static int readUnsignedIntFromStream(FILE* f)
{
    int result = 0;
    int shift = 0;
    while(true)
    {
        uint8_t buffer[1] = {0};
        if (fread(buffer, 1, 1, f) != 1)
            return -1;
        result |= int(buffer[0] & 0x7f) << shift;
        if (!(buffer[0] & 0x80))
            return result;
        shift += 7;
    }
}

bool Serializer::Handler::read(string name, ISerializable& object)
{
    if (mode == Mode::Unknown)
    {
        mode = Mode::Read;
        if (!readFile())
        {
            string_table.clear();
            data_blocks.clear();
            main_objects.clear();
            LOG(Warning, filename, "not a valid de-serialization file");
            return false;
        }
    }
    sp2assert(mode == Mode::Read, "Cannot mix writting and reading in a single serializer instance");

    const auto& string_table_it = string_table_lookup.find(name);
    if (string_table_it == string_table_lookup.end())
        return false;
    const auto& main_object_it = main_objects.find(string_table_it->second);
    if (main_object_it == main_objects.end())
        return false;
    
    auto data_block_it = data_blocks.begin();
    for(int n=0; n<main_object_it->second; n++)
    {
        ++data_block_it;
        if (data_block_it == data_blocks.end())
            return false;
    }
    current_data_block = &*data_block_it;
    buildKeyLookup();
    object.serialize(*this);
    return true;
}

bool Serializer::Handler::readFile()
{
    FILE* f = fopen(filename.c_str(), "rb");
    if (!f)
    {
        LOG(Warning, "Failed to open", filename, "for de-serialization");
        return false;
    }
    char buffer[8];
    if (fread(buffer, 8, 1, f) != 1)
        return false;
    if (memcmp(buffer, "SP2:data", 8) != 0)
        return false;
    int version = readUnsignedIntFromStream(f);
    if (version != 1)
        return false;
    int string_count = readUnsignedIntFromStream(f);
    if (string_count < 0)
        return false;
    int main_object_count = readUnsignedIntFromStream(f);
    if (main_object_count < 0)
        return false;
    int data_block_count = readUnsignedIntFromStream(f);
    if (data_block_count < 0)
        return false;
    for(int n=0; n<string_count; n++)
    {
        string s;
        int size = readUnsignedIntFromStream(f);
        if (size < 0)
            return false;
        s.resize(size);
        if (fread(&s[0], s.size(), 1, f) != 1)
            return false;
        string_table.emplace_back(s);
        string_table_lookup[s] = n;
    }
    for(int n=0; n<main_object_count; n++)
    {
        int string_table_index = readUnsignedIntFromStream(f);
        int data_block_index = readUnsignedIntFromStream(f);
        if (string_table_index < 0)
            return false;
        if (data_block_index < 0)
            return false;
        main_objects[string_table_index] = data_block_index;
    }
    for(int n=0; n<data_block_count; n++)
    {
        int size = readUnsignedIntFromStream(f);
        if (size < 0)
            return false;
        data_blocks.emplace_back();
        auto& block = data_blocks.back();
        block.resize(size);
        if (fread(block.data(), block.size(), 1, f) != 1)
            return false;
    }
    fclose(f);
    return true;
}

void Serializer::Handler::write(string name, ISerializable& object)
{
    if (mode == Mode::Unknown) mode = Mode::Write;
    sp2assert(mode == Mode::Write, "Cannot mix writting and reading in a single serializer instance");
    
    main_objects[addToStringTable(name)] = data_blocks.size();
    //Create our first data block to fill it with this object.
    data_blocks.emplace_back();
    current_data_block = &data_blocks.back();
    object.serialize(*this);
}

bool Serializer::Handler::buildKeyLookup()
{
    key_lookup.clear();
    for(read_index=0; read_index<int(current_data_block->size());)
    {
        int string_index = readUnsignedInt();
        key_lookup[string_table[string_index]] = read_index;
        skipType(readUnsignedInt());
    }
    read_index = 0;
    return true;
}

void Serializer::Handler::skipType(int type_id)
{
    if (read_index < 0) return;
    switch(type_id)
    {
    case Type_Int: readSignedInt(); break;
    case Type_Float: read_index += sizeof(float); break;
    case Type_Double: read_index += sizeof(double); break;
    case Type_Vector2f: read_index += sizeof(Vector2f); break;
    case Type_Vector2d: read_index += sizeof(Vector2d); break;
    case Type_Vector3f: read_index += sizeof(Vector3f); break;
    case Type_Vector3d: read_index += sizeof(Vector3d); break;
    case Type_String: readUnsignedInt(); break;
    case Type_Vector: {
        int vector_type_id = readUnsignedInt();
        int vector_size = readUnsignedInt();
        for(int n=0; n<vector_size; n++)
            skipType(vector_type_id);
        }break;
    case Type_Object: readUnsignedInt(); break;
    default:
        LOG(Warning, "Corrupted data in deserialization.");
        read_index = -1;
    }
}

void Serializer::Handler::processKey(const char* name)
{
    if (mode == Mode::Write)
    {
        addUnsignedInt(addToStringTable(name));
    }
    if (mode == Mode::Read)
    {
        const auto& it = key_lookup.find(name);
        if (it == key_lookup.end())
        {
            read_index = -1;
        }
        else
        {
            read_index = it->second;
        }
    }
}

void Serializer::Handler::processTypeID(int id)
{
    if (mode == Mode::Write)
    {
        addUnsignedInt(id);
    }
    else
    {
        if (read_index >= 0 && readUnsignedInt() != id)
            read_index = -1;
    }
}

void Serializer::Handler::processValue(string& value)
{
    if (mode == Mode::Write)
    {
        addUnsignedInt(addToStringTable(value));
    }
    if (mode == Mode::Read && read_index >= 0)
    {
        int index = readUnsignedInt();
        value = string_table[index];
    }
}

void Serializer::Handler::processValue(ISerializable& value)
{
    std::vector<uint8_t>* previous_data_block = current_data_block;
    
    if (mode == Mode::Write)
    {
        int index = data_blocks.size();

        data_blocks.emplace_back();
        current_data_block = &data_blocks.back();
        value.serialize(*this);

        current_data_block = previous_data_block;
        addUnsignedInt(index);
    }
    if (mode == Mode::Read && read_index >= 0)
    {
        int index = readUnsignedInt();
        int previous_read_index = read_index;
        std::map<string, int> string_lookup_backup(std::move(key_lookup));
        
        auto it = data_blocks.begin();
        for(int n=0; n<index; n++)
            ++it;
        current_data_block = &*it;
        buildKeyLookup();
        value.serialize(*this);

        current_data_block = previous_data_block;
        key_lookup = std::move(string_lookup_backup);
        read_index = previous_read_index;
    }
}

void Serializer::Handler::addUnsignedInt(int number)
{
    sp2assert(number >= 0, "Trying to add an unsigned negative number. Internal logic error?");
    sp2assert(mode == Mode::Write, "Tried to add data while not in write mode");
    while(number > 127)
    {
        current_data_block->push_back(0x80 | (number & 0x7f));
        number >>= 7;
    }
    current_data_block->push_back(number);
}

int Serializer::Handler::readUnsignedInt()
{
    sp2assert(mode == Mode::Read, "Tried to read data while not in read mode");
    sp2assert(read_index >= 0, "Tried to read data while no valid read pointer");
    int result = 0;
    int shift = 0;
    while((*current_data_block)[read_index] & 0x80)
    {
        result |= int((*current_data_block)[read_index++] & 0x7f) << shift;
        shift += 7;
    }
    result |= int((*current_data_block)[read_index++]) << shift;
    return result;
}

void Serializer::Handler::addSignedInt(int number)
{
    sp2assert(mode == Mode::Write, "Tried to add data while not in write mode");
    uint8_t neg_mask = 0;
    if (number < 0)
    {
        number = -number;
        neg_mask = 0x40;
    }
    if (number < 63)
    {
        current_data_block->push_back(number | neg_mask);
        return;
    }
    current_data_block->push_back(0x80 | neg_mask | (number & 0x3f));
    number >>= 6;
    while(number > 127)
    {
        current_data_block->push_back(0x80 | (number & 0x7f));
        number >>= 7;
    }
    current_data_block->push_back(number);
}

int Serializer::Handler::readSignedInt()
{
    sp2assert(mode == Mode::Read, "Tried to read data while not in read mode");
    sp2assert(read_index >= 0, "Tried to read data while no valid read pointer");
    int result = (*current_data_block)[read_index] & 0x3f;
    int shift = 6;
    bool neg = false;
    if ((*current_data_block)[read_index] & 0x40)
        neg = true;
    if ((*current_data_block)[read_index++] & 0x80)
    {
        while((*current_data_block)[read_index] & 0x80)
        {
            result |= int((*current_data_block)[read_index++] & 0x7f) << shift;
            shift += 7;
        }
        result |= int((*current_data_block)[read_index++]) << shift;
    }
    if (neg)
        result = -result;
    return result;
}

void Serializer::Handler::processDataBlock(void* data, int size)
{
    if (mode == Mode::Write)
    {
        int offset = current_data_block->size();
        current_data_block->resize(offset + size);
        memcpy(&(*current_data_block)[offset], data, size);
    }
    if (mode == Mode::Read)
    {
        if (read_index < 0)
            return;
        memcpy(data, &(*current_data_block)[read_index], size);
        read_index += size;
    }
}

int Serializer::Handler::addToStringTable(string str)
{
    const auto& it = string_table_lookup.find(str);
    if (it == string_table_lookup.end())
    {
        int result = string_table.size();
        string_table_lookup[str] = result;
        string_table.emplace_back(str);
        return result;
    }
    return it->second;
}

};//!namespace io
};//!namespace sp
