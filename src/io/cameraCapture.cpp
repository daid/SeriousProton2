#include <sp2/io/cameraCapture.h>

#if defined(__linux__)
#  include <unistd.h>
#  include <fcntl.h>
#  include <sys/ioctl.h>
#  include <sys/types.h>
#  include <sys/mman.h>
#  include <asm/types.h>
#  include <linux/videodev2.h>
#  include <string.h>
#endif//__linux__
#if defined(__WIN32)
#  include <dshow.h>
#  include <qedit.h>
DEFINE_GUID(CLSID_SampleGrabber,0xc1f400a0,0x3f08,0x11d3,0x9f,0x0b,0x00,0x60,0x08,0x03,0x9e,0x37);
DEFINE_GUID(CLSID_NullRenderer,0xc1f400a4,0x3f08,0x11d3,0x9f,0x0b,0x00,0x60,0x08,0x03,0x9e,0x37);
#endif//__WIN32


namespace sp {
namespace io {

#if defined(__linux__)

class CameraCapture::Data
{
public:
    Data() {}
    ~Data()
    {
        if (fd > -1)
            ::close(fd);
        if (buffer != MAP_FAILED)
            ::munmap(buffer, buffer_length);
    }

    int fd = -1;
    void* buffer = MAP_FAILED;
    size_t buffer_length;
};

bool CameraCapture::init(int index)
{
    data->fd = ::open(("/dev/video" + sp::string(index)).c_str(), O_RDWR);
    if (data->fd == -1)
    {
        LOG(Warning, "Failed to open /dev/video" + sp::string(index));
        return false;
    }
    
    struct v4l2_capability cap;
    if(::ioctl(data->fd, VIDIOC_QUERYCAP, &cap) < 0)
    {
        LOG(Error, "VIDIOC_QUERYCAP failed");
        return false;
    }
    
    if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        LOG(Error, "No V4L2_CAP_VIDEO_CAPTURE");
        return false;
    }
    
    size.x = 640;
    size.y = 480;

    struct v4l2_format format;
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    format.fmt.pix.width = size.x;
    format.fmt.pix.height = size.y;

    if(::ioctl(data->fd, VIDIOC_S_FMT, &format) < 0)
    {
        LOG(Error, "VIDIOC_S_FMT failed");
        return false;
    }
    
    struct v4l2_requestbuffers bufrequest;
    bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufrequest.memory = V4L2_MEMORY_MMAP;
    bufrequest.count = 1;

    if(::ioctl(data->fd, VIDIOC_REQBUFS, &bufrequest) < 0)
    {
        LOG(Error, "VIDIOC_REQBUFS failed");
        return false;
    }
    
    struct v4l2_buffer bufferinfo;
    memset(&bufferinfo, 0, sizeof(bufferinfo));     
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    bufferinfo.index = 0;
     
    if(::ioctl(data->fd, VIDIOC_QUERYBUF, &bufferinfo) < 0)
    {
        LOG(Error, "VIDIOC_QUERYBUF failed");
        return false;
    }
    
    data->buffer = ::mmap(nullptr, bufferinfo.length, PROT_READ | PROT_WRITE, MAP_SHARED, data->fd, bufferinfo.m.offset);
    data->buffer_length = bufferinfo.length;
    if(data->buffer == MAP_FAILED)
    {
        LOG(Error, "mmap failed");
        return false;
    }
    memset(data->buffer, 0, bufferinfo.length);

    memset(&bufferinfo, 0, sizeof(bufferinfo));
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    bufferinfo.index = 0;
     
    if(::ioctl(data->fd, VIDIOC_QBUF, &bufferinfo) < 0)
    {
        LOG(Error, "VIDIOC_QBUF failed");
        return false;
    }
    
    int type = bufferinfo.type;
    if(::ioctl(data->fd, VIDIOC_STREAMON, &type) < 0)
    {
        LOG(Error, "VIDIOC_STREAMON failed");
        return false;
    }

    return true;
}

class DataBufferResourceStream : public ResourceStream
{
private:
    void* buffer;
    size_t buffer_length;
    int64_t offset;
    
public:
    DataBufferResourceStream(void* buffer, size_t buffer_length)
    : buffer(buffer), buffer_length(buffer_length)
    {
        offset = 0;
    }
    
    virtual ~DataBufferResourceStream()
    {
    }
    
    virtual int64_t read(void* data, int64_t size)
    {
        size = std::min(int(buffer_length - offset), int(size));
        memcpy(data, ((char*)buffer) + offset, size);
        offset += size;
        return size;
    }
    
    virtual int64_t seek(int64_t position)
    {
        offset = std::max(0, std::min(int(buffer_length), int(position)));
        return offset;
    }
    
    virtual int64_t tell()
    {
        return offset;
    }
    
    virtual int64_t getSize()
    {
        return buffer_length;
    }
};

Image CameraCapture::getFrame()
{
    Image result;
    if (!data)
        return result;
    
    struct v4l2_buffer bufferinfo; 
    memset(&bufferinfo, 0, sizeof(bufferinfo));
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    bufferinfo.index = 0;
    
    //Wait till we have a frame.
    if(ioctl(data->fd, VIDIOC_DQBUF, &bufferinfo) < 0)
    {
        LOG(Warning, "VIDIOC_DQBUF failed");
        return result;
    }
    
    //As we have an JPG, we should be able to load it as resource stream.
    result.loadFromStream(std::make_shared<DataBufferResourceStream>(data->buffer, data->buffer_length));
    
    //Queue the buffer for next use, this starts the grab of the next frame.
    if(ioctl(data->fd, VIDIOC_QBUF, &bufferinfo) < 0)
    {
        LOG(Warning, "VIDIOC_QBUF failed");
    }
    return result;
}

#elif defined(__WIN32)

class CameraCapture::Data
{
public:
    Data() {}
    ~Data()
    {
        if (moniker) moniker->Release();
        if (capture_graph_builder) capture_graph_builder->Release();
        if (graph_builder) graph_builder->Release();
        if (media_control) media_control->Release();
        if (base_filter_cam) base_filter_cam->Release();
        if (base_filter_sample_grabber) base_filter_sample_grabber->Release();
        if (base_filter_null_renderer) base_filter_null_renderer->Release();
        if (AM_stream_config) AM_stream_config->Release();
        if (sample_grabber) sample_grabber->Release();
        if (media_event) media_event->Release();
        if (buffer) free(buffer);
    }
    
    IMoniker* moniker = nullptr;
    ICaptureGraphBuilder2* capture_graph_builder = nullptr;
    IGraphBuilder* graph_builder = nullptr;
    IMediaControl* media_control = nullptr;
    IBaseFilter* base_filter_cam = nullptr;
    IBaseFilter* base_filter_sample_grabber = nullptr;
    IBaseFilter* base_filter_null_renderer = nullptr;
    IAMStreamConfig* AM_stream_config = nullptr;
    ISampleGrabber* sample_grabber = nullptr;
    IMediaEventEx* media_event = nullptr;
    uint8_t* buffer = nullptr;
    long buffer_size = 0;
};

bool CameraCapture::init(int index)
{
    ICreateDevEnum* create_dev_enum = nullptr;
    HRESULT res;
    
    if (CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC, IID_ICreateDevEnum, (LPVOID*)&create_dev_enum))
        return false;
    
    IEnumMoniker *enum_moniker = nullptr;
    res = create_dev_enum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &enum_moniker, 0);
    if (res)
    {
        //Note: This happens if there are no cameras available.
        create_dev_enum->Release();
        return false;
    }
    
    while(enum_moniker->Next(1, &data->moniker, nullptr) == S_OK)
    {
        if (index == 0)
        {
            break;
        }
        index--;
        data->moniker->Release();
        data->moniker = nullptr;
    }
    enum_moniker->Release();
    create_dev_enum->Release();

    if (!data->moniker)
        return false;
    
    if (CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (LPVOID*)&data->capture_graph_builder))
        return false;
    if (CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (LPVOID*)&data->graph_builder))
        return false;
    if (data->graph_builder->QueryInterface(IID_IMediaControl, (LPVOID*)&data->media_control))
        return false;
    if (data->graph_builder->QueryInterface(IID_IMediaEventEx, (LPVOID*)&data->media_event))
        return false;

    if (data->capture_graph_builder->SetFiltergraph(data->graph_builder))
        return false;
    if (data->moniker->BindToObject(NULL, NULL, IID_IBaseFilter, (LPVOID*)&data->base_filter_cam))
        return false;
    if (data->graph_builder->AddFilter(data->base_filter_cam, L"VideoCam"))
        return false;

    CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (LPVOID*)&data->base_filter_sample_grabber);
    data->graph_builder->AddFilter(data->base_filter_sample_grabber, L"Sample Grabber");
    
    CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (LPVOID*)&data->base_filter_null_renderer);
    data->graph_builder->AddFilter(data->base_filter_null_renderer, L"NullRenderer");

    data->capture_graph_builder->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, data->base_filter_cam, data->base_filter_sample_grabber, data->base_filter_null_renderer);
    
    data->base_filter_sample_grabber->QueryInterface(IID_ISampleGrabber, (LPVOID*)&data->sample_grabber);
    data->sample_grabber->SetBufferSamples(true);
    
    data->capture_graph_builder->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, data->base_filter_cam, IID_IAMStreamConfig, (LPVOID*)&data->AM_stream_config);
    AM_MEDIA_TYPE* pAmMediaType = nullptr;
    data->AM_stream_config->GetFormat(&pAmMediaType);
    size.x = ((VIDEOINFOHEADER*)pAmMediaType->pbFormat)->bmiHeader.biWidth;
    size.y = ((VIDEOINFOHEADER*)pAmMediaType->pbFormat)->bmiHeader.biHeight;

    {
        if (pAmMediaType->pbFormat) CoTaskMemFree(pAmMediaType->pbFormat);
        if (pAmMediaType->pUnk) pAmMediaType->pUnk->Release();
        CoTaskMemFree(pAmMediaType);
    }

    AM_MEDIA_TYPE mt;
    ZeroMemory(&mt,sizeof(AM_MEDIA_TYPE));
    mt.majortype = MEDIATYPE_Video;
    mt.subtype = MEDIASUBTYPE_RGB24;
    mt.formattype = FORMAT_VideoInfo;
    data->sample_grabber->SetMediaType(&mt);

    data->media_control->Run();//For some reason, this reports failure, but the stream works...

    long buffer_size = -1;
    for(int n=0; n<10 && buffer_size == -1; n++)
    {
        if (data->sample_grabber->GetCurrentBuffer(&buffer_size, nullptr))
        {
            buffer_size = -1;
            Sleep(100);
        }
    }
    if (buffer_size == -1)
    {
        LOG(Warning, "Could open camera, but no images are captured after 1 second.");
        return false;
    }
    return true;
}

Image CameraCapture::getFrame()
{
    Image result;
    if (!data)
        return result;

    long buffer_size = -1;
    if (data->sample_grabber->GetCurrentBuffer(&buffer_size, nullptr))
        return result;
    buffer_size = buffer_size * 4 / 3;
    if (data->buffer_size != buffer_size)
    {
        if (data->buffer) free(data->buffer);
        data->buffer = (uint8_t*)malloc(buffer_size);
        data->buffer_size = buffer_size;
    }
    if (data->sample_grabber->GetCurrentBuffer(&buffer_size, (LONG*)data->buffer))
        return result;
    
    //In place convert 24bit BGR into 24bit RGBA.
    uint8_t* src = data->buffer + (size.x * size.y) * 3;
    uint32_t* dst = (uint32_t*)(data->buffer) + (size.x * size.y);
    for(int n=0; n<size.x*size.y; n++)
    {
        src -= 3;
        dst -= 1;
        *dst = 0xff000000 | src[2] | (src[1] << 8) | (src[0] << 16);
    }
    
    //Flip upsize down.
    uint32_t tmp[size.x];
    for(int n=0; n<size.y / 2; n++)
    {
        memcpy(tmp, data->buffer + size.x * n * 4, size.x * 4);
        memcpy(data->buffer + size.x * n * 4, data->buffer + size.x * (size.y - 1 - n) * 4, size.x * 4);
        memcpy(data->buffer + size.x * (size.y - 1 - n) * 4, tmp, size.x * 4);
    }
    
    result.update(size, (uint32_t*)data->buffer);
    return result;
}

#else

/** Dummy implementation if we do not have an OS specific implementation. */    
class CameraCapture::Data
{
public:
    Data() {}
    ~Data() {}
};

bool CameraCapture::init(int index)
{
    return false;
}

Image CameraCapture::getFrame()
{
    return Image();
}

#endif

// Shared CameraCapture implementation
CameraCapture::CameraCapture(int index)
: data(nullptr)
{
    data = new Data();
    if (!init(index))
    {
        LOG(Warning, "Failed to open camera:", index);
        delete data;
        data = nullptr;
    }
}

CameraCapture::~CameraCapture()
{
    if (data)
        delete data;
}

bool CameraCapture::isOpen()
{
    return data != nullptr;
}

};//namespace io
};//namespace sp
