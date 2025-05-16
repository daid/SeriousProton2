#include <sp2/io/bufferResourceStream.h>

namespace sp {
namespace io {
    
DataBufferResourceStream::DataBufferResourceStream(void* buffer, size_t buffer_length)
: buffer(buffer), buffer_length(buffer_length)
{
    offset = 0;
}

DataBufferResourceStream::~DataBufferResourceStream()
{
}

int64_t DataBufferResourceStream::read(void* data, int64_t size)
{
    size = std::min(int(buffer_length - offset), int(size));
    memcpy(data, static_cast<char*>(buffer) + offset, size);
    offset += size;
    return size;
}

int64_t DataBufferResourceStream::seek(int64_t position)
{
    offset = std::max(0, std::min(int(buffer_length), int(position)));
    return offset;
}

int64_t DataBufferResourceStream::tell()
{
    return offset;
}

int64_t DataBufferResourceStream::getSize()
{
    return buffer_length;
}

}//namespace io
}//namespace sp
