#ifndef SP2_IO_BUFFERRESOURCESTREAM_H
#define SP2_IO_BUFFERRESOURCESTREAM_H

#include <sp2/io/resourceProvider.h>

namespace sp {
namespace io {

class DataBufferResourceStream : public ResourceStream
{
private:
    void* buffer;
    size_t buffer_length;
    int64_t offset;
    
public:
    DataBufferResourceStream(void* buffer, size_t buffer_length);
    virtual ~DataBufferResourceStream();
    
    virtual int64_t read(void* data, int64_t size) override;
    virtual int64_t seek(int64_t position) override;
    virtual int64_t tell() override;
    virtual int64_t getSize() override;
};

}//namespace io
}//namespace sp

#endif//SP2_IO_BUFFERRESOURCESTREAM_H