#include <sp2/io/zipResourceProvider.h>
#include <sp2/attributes.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "miniz.h"


namespace sp {
namespace io {

struct ZipEOCD
{
    uint32_t signature;
    uint16_t disk_nr;
    uint16_t central_directory_disk;
    uint16_t central_directory_records_on_this_disk;
    uint16_t central_directory_records_total;
    uint32_t central_directory_size;
    uint32_t central_directory_offset;
    uint16_t comment_length;
} SP2_PACKED;

struct ZipCentralDirectory
{
    uint32_t signature;
    uint16_t created_by_version;
    uint16_t minimal_required_version;
    uint16_t flags;
    uint16_t method;
    uint16_t mtime;
    uint16_t mdate;
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t filename_length;
    uint16_t extra_field_length;
    uint16_t comment_length;
    uint16_t disk_nr;
    uint16_t internal_file_attributes;
    uint32_t external_file_attributes;
    uint32_t offset_local_file_header;
} SP2_PACKED;

struct ZipLocalHeader
{
    uint32_t signature;
    uint16_t minimal_required_version;
    uint16_t flags;
    uint16_t method;
    uint16_t mtime;
    uint16_t mdate;
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t filename_length;
    uint16_t extra_field_length;
} SP2_PACKED;
static_assert(sizeof(ZipLocalHeader) == 30, "");

class ZipStoredResourceStream : public ResourceStream
{
private:
    FILE* f;
    uint64_t offset;
    uint64_t size;

    uint64_t current_offset;

public:
    ZipStoredResourceStream(const string& zip_filename, uint64_t offset, uint64_t size)
    : offset(offset), size(size), current_offset(0)
    {
        f = fopen(zip_filename.c_str(), "rb");
        fseek(f, offset, SEEK_SET);
    }
    ~ZipStoredResourceStream()
    {
        if (f)
            fclose(f);
    }

    virtual int64_t read(void* data, int64_t size) override
    {
        if (current_offset + size > this->size)
            size = this->size - current_offset;
        int64_t result = fread(data, 1, size, f);
        if (result < 0)
            result = 0;
        current_offset += result;
        return result;
    }

    virtual int64_t seek(int64_t position) override
    {
        if (position < 0)
            position = 0;
        if (position > int64_t(size))
            position = size;
        fseek(f, offset + position, SEEK_SET);
        current_offset = position;
        return current_offset;
    }

    virtual int64_t tell() override
    {
        return current_offset;
    }

    virtual int64_t getSize() override
    {
        return size;
    }
};

class ZipDeflateResourceStream : public ResourceStream
{
private:
    FILE* f;
    uint64_t offset;
    uint64_t uncompressed_size;

    mz_stream stream;
    uint8_t input_buffer[16 * 1024];
    uint8_t output_buffer[16 * 1024];
    int64_t uncompressed_offset;
public:
    ZipDeflateResourceStream(const string& zip_filename, uint64_t offset, uint64_t uncompressed_size)
    : offset(offset), uncompressed_size(uncompressed_size)
    {
        f = fopen(zip_filename.c_str(), "rb");
        fseek(f, offset, SEEK_SET);
        
        stream.zalloc = NULL;
        stream.zfree = NULL;
        stream.opaque = NULL;
        stream.avail_in = 0;
        stream.next_in = NULL;
        mz_inflateInit2(&stream, -15);
        
        stream.avail_out = sizeof(output_buffer);
        stream.next_out = output_buffer;
        
        uncompressed_offset = 0;
    }
    virtual ~ZipDeflateResourceStream()
    {
        mz_inflateEnd(&stream);
        if (f)
            fclose(f);
    }
    
    virtual int64_t read(void* data, int64_t size) override
    {
        int available = stream.next_out - output_buffer;
        int copy = std::min(int(size), available);
        if (copy)
        {
            uncompressed_offset += copy;
            if (data)
                memcpy(data, output_buffer, copy);
            stream.avail_out += copy;
            memmove(output_buffer, output_buffer + copy, available - copy);
            stream.next_out -= copy;
            if (copy == size)
                return size;
            if (data)
                data = static_cast<uint8_t*>(data) + copy;
            size -= copy;
        }
        //Not enough data in output_buffer, inflate some more data.
        if (stream.avail_in == 0)
        {
            stream.avail_in = fread(input_buffer, 1, sizeof(input_buffer), f);
            if (stream.avail_in <= 0)
            {
                stream.avail_in = 0;
                return copy;
            }
            stream.next_in = input_buffer;
        }
        int result = mz_inflate(&stream, MZ_NO_FLUSH);
        if ( (result != MZ_OK && result != MZ_STREAM_END) || (stream.next_out == output_buffer) )
        {
            //Stream decode error, no data available while we expect it.
            return copy;
        }
        return copy + read(data, size);
    }

    virtual int64_t seek(int64_t position) override
    {
        if (position > uncompressed_offset)
        {
            //Seeking forwards, just read data into nothing to get there.
            read(nullptr, position - uncompressed_offset);
        }
        else if (position < uncompressed_offset)
        {
            //When we seek backwards, the only thing we can do is restart from the beginning.
            mz_inflateEnd(&stream);

            fseek(f, offset, SEEK_SET);
            stream.zalloc = NULL;
            stream.zfree = NULL;
            stream.opaque = NULL;
            stream.avail_in = 0;
            stream.next_in = NULL;
            mz_inflateInit2(&stream, -15);
        
            stream.avail_out = sizeof(output_buffer);
            stream.next_out = output_buffer;
            uncompressed_offset = 0;
            return seek(position);
        }
        return tell();
    }
    
    virtual int64_t tell() override
    {
        return uncompressed_offset;
    }
    
    virtual int64_t getSize() override
    {
        return uncompressed_size;
    }
};


ZipResourceProvider::ZipResourceProvider(const string& zip_filename, int priority)
: ResourceProvider(priority), zip_filename(zip_filename)
{
    FILE* f = fopen(zip_filename.c_str(), "rb");
    if (!f)
    {
        LOG(Warning, "Zip file not found:", zip_filename);
        return;
    }
    fseek(f, -22, SEEK_END);
    ZipEOCD eocd;
    if (fread(&eocd, 22, 1, f) != 1)
    {
        fclose(f);
        LOG(Warning, "Corrupt zip file, failed to read EOCD:", zip_filename);
        return;
    }
    if (eocd.signature != 0x06054b50 || eocd.disk_nr != 0 || eocd.central_directory_disk != 0 || eocd.central_directory_records_on_this_disk != eocd.central_directory_records_total || eocd.comment_length != 0)
    {
        fclose(f);
        LOG(Warning, "Corrupt zip file, failed to parse EOCD:", zip_filename, "(zip might contain global comment, which is not supported)");
        return;
    }

    fseek(f, eocd.central_directory_offset, SEEK_SET);
    while(eocd.central_directory_size > 0)
    {
        ZipCentralDirectory central_directory;
        if (fread(&central_directory, 46, 1, f) != 1)
        {
            fclose(f);
            LOG(Warning, "Corrupt zip file, failed to read central directory record:", zip_filename);
            contents.clear();
            return;
        }
        eocd.central_directory_size -= 46;
        if (central_directory.signature != 0x02014b50 || central_directory.disk_nr != 0)
        {
            fclose(f);
            LOG(Warning, "Corrupt zip file, failed to parse central directory record:", zip_filename);
            contents.clear();
            return;
        }
        if (central_directory.method != 0 && central_directory.method != 8)
        {
            fclose(f);
            LOG(Warning, "Zip file contains compressed files not using STORE or DEFLATE compression. Only STORE or DEFLATE is supported:", zip_filename, "Method:", central_directory.method);
            contents.clear();
            return;
        }
        eocd.central_directory_size -= central_directory.filename_length;
        eocd.central_directory_size -= central_directory.extra_field_length;
        eocd.central_directory_size -= central_directory.comment_length;

        string filename;
        filename.resize(central_directory.filename_length);
        if (fread(&filename[0], central_directory.filename_length, 1, f) != 1)
        {
            fclose(f);
            LOG(Warning, "Corrupt zip file, failed to read filename in central directory record:", zip_filename);
            contents.clear();
            return;
        }

        //TODO: Read extra field data for ZIP64 support.
        fseek(f, central_directory.extra_field_length, SEEK_CUR);
        fseek(f, central_directory.comment_length, SEEK_CUR);

        contents[filename].method = central_directory.method;
        contents[filename].offset = central_directory.offset_local_file_header;
        contents[filename].compressed_size = central_directory.compressed_size;
        contents[filename].uncompressed_size = central_directory.uncompressed_size;
    }
    for(auto& it : contents)
    {
        ZipLocalHeader local_header;
        fseek(f, it.second.offset, SEEK_SET);
        if (fread(&local_header, 30, 1, f) != 1)
        {
            fclose(f);
            LOG(Warning, "Corrupt zip file, failed to read local header:", zip_filename, it.first, "@", it.second.offset);
            contents.clear();
            return;
        }
        if (local_header.signature != 0x04034b50)
        {
            fclose(f);
            LOG(Warning, "Corrupt zip file, failed to parse local header:", zip_filename, it.first, "@", it.second.offset);
            contents.clear();
            return;
        }
        it.second.offset += 30 + local_header.filename_length + local_header.extra_field_length;
    }
    fclose(f);
}

ResourceStreamPtr ZipResourceProvider::getStream(const string& filename)
{
    auto it = contents.find(filename);
    if (it != contents.end())
    {
        if (it->second.method == 0)
            return std::make_shared<ZipStoredResourceStream>(zip_filename, it->second.offset, it->second.uncompressed_size);
        else
            return std::make_shared<ZipDeflateResourceStream>(zip_filename, it->second.offset, it->second.uncompressed_size);
    }
    return nullptr;
}

std::chrono::system_clock::time_point ZipResourceProvider::getResourceModifyTime(const string& filename)
{
    return std::chrono::system_clock::time_point();
}

std::vector<string> ZipResourceProvider::findResources(const string& search_pattern)
{
    std::vector<string> found_files;
    return found_files;
}

}//namespace io
}//namespace sp
