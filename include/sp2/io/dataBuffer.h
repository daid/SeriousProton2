#ifndef SP2_IO_DATABUFFER_H
#define SP2_IO_DATABUFFER_H

#include <sp2/string.h>

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#  define IF_LITTLE_ENDIAN(x) do { x; } while(0)
#else
#  define IF_LITTLE_ENDIAN(x) do { } while(0)
#endif

namespace sp {
namespace io {

class DataBuffer
{
public:
    DataBuffer()
    : read_index(0)
    {
    }
    
    template<typename... ARGS> DataBuffer(const ARGS&... args)
    : DataBuffer()
    {
        write(args...);
    }
    
    void operator=(std::vector<uint8_t>&& data)
    {
         buffer = std::move(data);
         read_index = 0;
    }

    void clear()
    {
        buffer.clear();
        read_index = 0;
    }
    
    const void* getData() const
    {
        return buffer.data();
    }
    
    unsigned int getDataSize() const
    {
        return buffer.size();
    }
    
    template<typename T, typename... ARGS> void write(const T& value, const ARGS&... args)
    {
        write(value);
        write(args...);
    }
    
    void write(bool b)
    {
        buffer.push_back(b ? 1 : 0);
    }
    
    void write(uint8_t i)
    {
        buffer.push_back(i);
    }

    void write(int8_t i)
    {
        buffer.push_back(i);
    }

    void write(int16_t i) { write(uint16_t(i)); }
    void write(int32_t i) { write(uint32_t(i)); }
    void write(int64_t i) { write(uint64_t(i)); }
    void write(uint16_t i)
    {
        IF_LITTLE_ENDIAN(i = __builtin_bswap16(i));
        size_t idx = buffer.size();
        buffer.resize(idx + sizeof(i));
        memcpy(&buffer[idx], &i, sizeof(i));
    }
    
    void write(uint32_t i)
    {
        IF_LITTLE_ENDIAN(i = __builtin_bswap32(i));
        size_t idx = buffer.size();
        buffer.resize(idx + sizeof(i));
        memcpy(&buffer[idx], &i, sizeof(i));
    }

    void write(uint64_t i)
    {
        IF_LITTLE_ENDIAN(i = __builtin_bswap64(i));
        size_t idx = buffer.size();
        buffer.resize(idx + sizeof(i));
        memcpy(&buffer[idx], &i, sizeof(i));
    }

    void write(const float f)
    {
        size_t idx = buffer.size();
        buffer.resize(idx + sizeof(f));
        memcpy(&buffer[idx], &f, sizeof(f));
    }

    void write(const double f)
    {
        size_t idx = buffer.size();
        buffer.resize(idx + sizeof(f));
        memcpy(&buffer[idx], &f, sizeof(f));
    }

    void write(const string& s)
    {
        write(uint32_t(s.length()));
        size_t idx = buffer.size();
        buffer.resize(idx + s.length());
        memcpy(&buffer[idx], &s[0], s.length());
    }
    
    template<typename T, typename... ARGS> void read(T& value, ARGS&... args)
    {
        read(value);
        read(args...);
    }

    void read(bool& b)
    {
        if (read_index >= buffer.size()) return;
        b = buffer[read_index++];
    }
    
    void read(uint8_t& i)
    {
        if (read_index >= buffer.size()) return;
        i = buffer[read_index++];
    }

    void read(int8_t& i)
    {
        if (read_index >= buffer.size()) return;
        i = buffer[read_index++];
    }

    void read(int16_t& i)
    {
        if (read_index > buffer.size() - sizeof(i)) return;
        memcpy(&buffer[read_index], &i, sizeof(i));
        read_index += sizeof(i);
        IF_LITTLE_ENDIAN(i = __builtin_bswap16(i));
    }
    void read(int32_t& i)
    {
        if (read_index > buffer.size() - sizeof(i)) return;
        memcpy(&buffer[read_index], &i, sizeof(i));
        read_index += sizeof(i);
        IF_LITTLE_ENDIAN(i = __builtin_bswap32(i));
    }
    
    void read(int64_t& i)
    {
        if (read_index > buffer.size() - sizeof(i)) return;
        memcpy(&buffer[read_index], &i, sizeof(i));
        read_index += sizeof(i);
        IF_LITTLE_ENDIAN(i = __builtin_bswap64(i));
    }
    
    void read(uint16_t& i)
    {
        if (read_index > buffer.size() - sizeof(i)) return;
        memcpy(&buffer[read_index], &i, sizeof(i));
        read_index += sizeof(i);
        IF_LITTLE_ENDIAN(i = __builtin_bswap16(i));
    }
    
    void read(uint32_t& i)
    {
        if (read_index > buffer.size() - sizeof(i)) return;
        memcpy(&buffer[read_index], &i, sizeof(i));
        read_index += sizeof(i);
        IF_LITTLE_ENDIAN(i = __builtin_bswap32(i));
    }

    void read(uint64_t& i)
    {
        if (read_index > buffer.size() - sizeof(i)) return;
        memcpy(&buffer[read_index], &i, sizeof(i));
        read_index += sizeof(i);
        IF_LITTLE_ENDIAN(i = __builtin_bswap64(i));
    }

    void read(float& f)
    {
        if (read_index > buffer.size() - sizeof(f)) return;
        memcpy(&buffer[read_index], &f, sizeof(f));
        read_index += sizeof(f);
    }

    void read(double& f)
    {
        if (read_index > buffer.size() - sizeof(f)) return;
        memcpy(&buffer[read_index], &f, sizeof(f));
        read_index += sizeof(f);
    }
    
    void read(string& s)
    {
        uint32_t len = 0;
        read(len);
        if (read_index > buffer.size() - len) return;
        s.assign((char*)&buffer[read_index], len);
        read_index += len;
    }
    
    size_t available()
    {
        return buffer.size() - read_index;
    }
private:
    std::vector<uint8_t> buffer;
    size_t read_index;
};

};//namespace io
};//namespace sp

#undef IF_LITTLE_ENDIAN

#endif//SP2_IO_DATABUFFER_H
