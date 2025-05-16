#include <sp2/io/cameraCapture.h>
#include <sp2/io/bufferResourceStream.h>

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
#if defined(_WIN32)
#  include <dshow.h>
#  include <qedit.h>
DEFINE_GUID(CLSID_SampleGrabber,0xc1f400a0,0x3f08,0x11d3,0x9f,0x0b,0x00,0x60,0x08,0x03,0x9e,0x37);
DEFINE_GUID(CLSID_NullRenderer,0xc1f400a4,0x3f08,0x11d3,0x9f,0x0b,0x00,0x60,0x08,0x03,0x9e,0x37);
#endif//_WIN32
#if defined(__EMSCRIPTEN__)
#  include <emscripten.h>
#endif

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

CameraCapture::State CameraCapture::init(int index, sp::Vector2i prefered_size)
{
    data->fd = ::open(("/dev/video" + string(index)).c_str(), O_RDWR);
    if (data->fd == -1)
    {
        LOG(Warning, "Failed to open /dev/video" + string(index));
        return State::Closed;
    }
    
    struct v4l2_capability cap;
    if(::ioctl(data->fd, VIDIOC_QUERYCAP, &cap) < 0)
    {
        LOG(Error, "VIDIOC_QUERYCAP failed");
        return State::Closed;
    }
    
    if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        LOG(Error, "No V4L2_CAP_VIDEO_CAPTURE");
        return State::Closed;
    }
    
    size.x = 640;
    size.y = 480;
    if (prefered_size.x && prefered_size.y)
        size = prefered_size;

    struct v4l2_format format;
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    format.fmt.pix.width = size.x;
    format.fmt.pix.height = size.y;

    if(::ioctl(data->fd, VIDIOC_S_FMT, &format) < 0)
    {
        LOG(Error, "VIDIOC_S_FMT failed");
        return State::Closed;
    }
    
    struct v4l2_requestbuffers bufrequest;
    bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufrequest.memory = V4L2_MEMORY_MMAP;
    bufrequest.count = 1;

    if(::ioctl(data->fd, VIDIOC_REQBUFS, &bufrequest) < 0)
    {
        LOG(Error, "VIDIOC_REQBUFS failed");
        return State::Closed;
    }
    
    struct v4l2_buffer bufferinfo;
    memset(&bufferinfo, 0, sizeof(bufferinfo));     
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    bufferinfo.index = 0;
     
    if(::ioctl(data->fd, VIDIOC_QUERYBUF, &bufferinfo) < 0)
    {
        LOG(Error, "VIDIOC_QUERYBUF failed");
        return State::Closed;
    }
    
    data->buffer = ::mmap(nullptr, bufferinfo.length, PROT_READ | PROT_WRITE, MAP_SHARED, data->fd, bufferinfo.m.offset);
    data->buffer_length = bufferinfo.length;
    if(data->buffer == MAP_FAILED)
    {
        LOG(Error, "mmap failed");
        return State::Closed;
    }
    memset(data->buffer, 0, bufferinfo.length);

    memset(&bufferinfo, 0, sizeof(bufferinfo));
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    bufferinfo.index = 0;
     
    if(::ioctl(data->fd, VIDIOC_QBUF, &bufferinfo) < 0)
    {
        LOG(Error, "VIDIOC_QBUF failed");
        return State::Closed;
    }
    
    int type = bufferinfo.type;
    if(::ioctl(data->fd, VIDIOC_STREAMON, &type) < 0)
    {
        LOG(Error, "VIDIOC_STREAMON failed");
        return State::Closed;
    }

    int flags = ::fcntl(data->fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    ::fcntl(data->fd, F_SETFL, flags);

    return State::Streaming;
}

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
    
    //Grab a frame if possible.
    if(ioctl(data->fd, VIDIOC_DQBUF, &bufferinfo) < 0)
    {
        if (errno == EAGAIN || errno == EINPROGRESS || errno == EWOULDBLOCK)
            return result;
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

CameraCapture::State CameraCapture::getState()
{
    return state;
}

std::vector<CameraCapture::Control> CameraCapture::getControls()
{
    if (!data)
        return {};
    std::vector<Control> results;

    struct v4l2_queryctrl queryctrl;
    struct v4l2_control control;
    memset(&queryctrl, 0, sizeof(queryctrl));
    queryctrl.id = V4L2_CTRL_CLASS_USER | V4L2_CTRL_FLAG_NEXT_CTRL;
    while (0 == ioctl(data->fd, VIDIOC_QUERYCTRL, &queryctrl)) {
        if (V4L2_CTRL_ID2CLASS(queryctrl.id) != V4L2_CTRL_CLASS_USER)
            break;
        if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
            continue;

        //if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
        //    enumerate_menu();
        control.id = queryctrl.id;
        ioctl(data->fd, VIDIOC_G_CTRL, &control);
        results.push_back({string(reinterpret_cast<const char*>(queryctrl.name)), control.value, queryctrl.minimum, queryctrl.maximum});

        queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
    }

    return results;
}

void CameraCapture::setControl(const string& name, int value)
{
    if (!data)
        return;

    struct v4l2_queryctrl queryctrl;
    struct v4l2_control control;
    memset(&queryctrl, 0, sizeof(queryctrl));
    queryctrl.id = V4L2_CTRL_CLASS_USER | V4L2_CTRL_FLAG_NEXT_CTRL;
    while (0 == ioctl(data->fd, VIDIOC_QUERYCTRL, &queryctrl)) {
        if (V4L2_CTRL_ID2CLASS(queryctrl.id) != V4L2_CTRL_CLASS_USER)
            break;
        if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
            continue;

        if (string(reinterpret_cast<const char*>(queryctrl.name)) == name) {
            control.id = queryctrl.id;
            ioctl(data->fd, VIDIOC_G_CTRL, &control);
            control.value = value;
            ioctl(data->fd, VIDIOC_S_CTRL, &control);
        }

        queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
    }
}

#elif defined(_WIN32)

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
    
    enum class Type
    {
        MJPG,
        RGB24,
        YUY2,
        NV12,
    } type;
};

CameraCapture::State CameraCapture::init(int index, sp::Vector2i prefered_size)
{
    ICreateDevEnum* create_dev_enum = nullptr;
    HRESULT res;
    
    if (CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC, IID_ICreateDevEnum, reinterpret_cast<LPVOID*>(&create_dev_enum)))
        return State::Closed;
    
    IEnumMoniker *enum_moniker = nullptr;
    res = create_dev_enum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &enum_moniker, 0);
    if (res)
    {
        //Note: This happens if there are no cameras available.
        create_dev_enum->Release();
        return State::Closed;
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
        return State::Closed;
    
    if (CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, reinterpret_cast<LPVOID*>(&data->capture_graph_builder)))
        return State::Closed;
    if (CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, reinterpret_cast<LPVOID*>(&data->graph_builder)))
        return State::Closed;
    if (data->graph_builder->QueryInterface(IID_IMediaControl, reinterpret_cast<LPVOID*>(&data->media_control)))
        return State::Closed;
    if (data->graph_builder->QueryInterface(IID_IMediaEventEx, reinterpret_cast<LPVOID*>(&data->media_event)))
        return State::Closed;

    if (data->capture_graph_builder->SetFiltergraph(data->graph_builder))
        return State::Closed;
    if (data->moniker->BindToObject(NULL, NULL, IID_IBaseFilter, reinterpret_cast<LPVOID*>(&data->base_filter_cam)))
        return State::Closed;
    if (data->graph_builder->AddFilter(data->base_filter_cam, L"VideoCam"))
        return State::Closed;

    CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<LPVOID*>(&data->base_filter_sample_grabber));
    data->graph_builder->AddFilter(data->base_filter_sample_grabber, L"Sample Grabber");
    
    CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<LPVOID*>(&data->base_filter_null_renderer));
    data->graph_builder->AddFilter(data->base_filter_null_renderer, L"NullRenderer");

    data->capture_graph_builder->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, data->base_filter_cam, data->base_filter_sample_grabber, data->base_filter_null_renderer);
    
    data->base_filter_sample_grabber->QueryInterface(IID_ISampleGrabber, reinterpret_cast<LPVOID*>(&data->sample_grabber));
    data->sample_grabber->SetBufferSamples(true);
    
    data->capture_graph_builder->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, data->base_filter_cam, IID_IAMStreamConfig, reinterpret_cast<LPVOID*>(&data->AM_stream_config));
    AM_MEDIA_TYPE* pAmMediaType = nullptr;
    data->AM_stream_config->GetFormat(&pAmMediaType);
    size.x = reinterpret_cast<VIDEOINFOHEADER*>(pAmMediaType->pbFormat)->bmiHeader.biWidth;
    size.y = reinterpret_cast<VIDEOINFOHEADER*>(pAmMediaType->pbFormat)->bmiHeader.biHeight;

    {
        if (pAmMediaType->pbFormat) CoTaskMemFree(pAmMediaType->pbFormat);
        if (pAmMediaType->pUnk) pAmMediaType->pUnk->Release();
        CoTaskMemFree(pAmMediaType);
    }

    AM_MEDIA_TYPE mt;
    ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
    mt.majortype = MEDIATYPE_Video;
    mt.subtype = MEDIASUBTYPE_RGB24;
    mt.formattype = FORMAT_VideoInfo;
    data->sample_grabber->SetMediaType(&mt);
    data->sample_grabber->GetConnectedMediaType(&mt);
    if (mt.pbFormat) CoTaskMemFree(mt.pbFormat);
    if (mt.pUnk) mt.pUnk->Release();
    if (mt.subtype == MEDIASUBTYPE_MJPG)
    {
        data->type = Data::Type::MJPG;
    }
    else if (mt.subtype == MEDIASUBTYPE_RGB24)
    {
        data->type = Data::Type::RGB24;
    }
    else if (mt.subtype == MEDIASUBTYPE_YUY2)
    {
        data->type = Data::Type::YUY2;
    }
    else if (mt.subtype == MEDIASUBTYPE_NV12)
    {
        data->type = Data::Type::NV12;
    }
    else
    {
        LOG(Error, "Unknown MEDIASUBTYPE in camera settings: ", sp::string::hex(mt.subtype.Data1));
        return State::Closed;
    }

    auto result = data->media_control->Run();
    if (FAILED(result))
    {
        LOG(Warning, "Could open camera, but failed to start the stream.");
        return State::Closed;
    }

/*
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
*/
    return State::Streaming;
}

Image CameraCapture::getFrame()
{
    Image result;
    if (!data)
        return result;

    long buffer_size = -1;
    if (data->sample_grabber->GetCurrentBuffer(&buffer_size, nullptr))
        return result;
    buffer_size = std::max(buffer_size, long(size.x*size.y*4));
    if (data->buffer_size != buffer_size)
    {
        data->buffer = static_cast<uint8_t*>(malloc(buffer_size));
        data->buffer_size = buffer_size;
    }
    if (data->sample_grabber->GetCurrentBuffer(&buffer_size, reinterpret_cast<LONG*>(data->buffer)))
        return result;
    
    switch(data->type)
    {
    case Data::Type::MJPG:
        //As we have an JPG, we should be able to load it as resource stream.
        result.loadFromStream(std::make_shared<DataBufferResourceStream>(data->buffer, buffer_size));
        break;
    case Data::Type::RGB24:{
        if (buffer_size != 3 * size.x * size.y)
        {
            LOG(Warning, "Incorrect buffer size result on CameraCapture:", buffer_size, 3 * size.x * size.y);
            return result;
        }

        //In place convert 24bit BGR into 24bit RGBA.
        uint8_t* src = data->buffer + (size.x * size.y) * 3;
        uint32_t* dst = reinterpret_cast<uint32_t*>(data->buffer) + (size.x * size.y);
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
        result.update(size, reinterpret_cast<uint32_t*>(data->buffer));
        }break;
    case Data::Type::NV12:{
        if (buffer_size != size.x * size.y + (size.x * size.y / 2))
        {
            LOG(Warning, "Incorrect buffer size result on CameraCapture:", buffer_size, 2 * size.x * size.y);
            return result;
        }
        uint8_t* ptr_y = data->buffer;
        uint8_t* ptr_u = data->buffer + size.x*size.y;
        uint8_t* ptr_v = data->buffer + size.x*size.y + 1;

        //TODO: This still corrupts the top part of the image.
        for(int py=size.y-1; py>=0; py--)
        for(int px=0; px<size.x; px++)
        {
            int y = ptr_y[px + py * size.x];
            int u = ptr_u[(px/2 + py/2 * size.x/2)*2];
            int v = ptr_v[(px/2 + py/2 * size.x/2)*2];

            u -= 128;
            v -= 128;

            data->buffer[(px+py*size.x)*4+0] = std::clamp(int(y + 1.402 * v), 0, 255);
            data->buffer[(px+py*size.x)*4+1] = std::clamp(int(y - 0.34414 * u - 0.71414 * v), 0, 255);
            data->buffer[(px+py*size.x)*4+2] = std::clamp(int(y + 1.772 * u), 0, 255);
            data->buffer[(px+py*size.x)*4+3] = 255;
        }
        result.update(size, reinterpret_cast<uint32_t*>(data->buffer));
        }break;
    case Data::Type::YUY2:{
        if (buffer_size != 2 * size.x * size.y)
        {
            LOG(Warning, "Incorrect buffer size result on CameraCapture:", buffer_size, 2 * size.x * size.y);
            return result;
        }

        uint8_t* src = data->buffer + (size.x * size.y) * 2;
        uint8_t* dst = data->buffer + (size.x * size.y) * 4;
        for(int n=0; n<size.x*size.y; n+=2)
        {
            src -= 4;
            dst -= 8;

            int y0 = src[0];
            int u0 = src[1];
            int y1 = src[2];
            int v0 = src[3];

            int c = y0 - 16;
            int d = u0 - 128;
            int e = v0 - 128;
            dst[0] = std::min(255, std::max(0, ( 298 * c + 409 * e + 128) >> 8)); // red
            dst[1] = std::min(255, std::max(0, ( 298 * c - 100 * d - 208 * e + 128) >> 8)); // green
            dst[2] = std::min(255, std::max(0, ( 298 * c + 516 * d + 128) >> 8)); // blue
            dst[3] = 0xff;
            c = y1 - 16;
            dst[4] = std::min(255, std::max(0, ( 298 * c + 409 * e + 128) >> 8)); // red
            dst[5] = std::min(255, std::max(0, ( 298 * c - 100 * d - 208 * e + 128) >> 8)); // green
            dst[6] = std::min(255, std::max(0, ( 298 * c + 516 * d + 128) >> 8)); // blue
            dst[7] = 0xff;
        }
        
        result.update(size, reinterpret_cast<uint32_t*>(data->buffer));
        }break;
    }
    return result;
}

std::vector<CameraCapture::Control> CameraCapture::getControls()
{
    if (!data)
        return {};
    std::vector<Control> results;

    IAMVideoProcAmp* pProcAmp = nullptr;
    if (data->base_filter_cam->QueryInterface(IID_IAMVideoProcAmp, reinterpret_cast<void**>(&pProcAmp)) >= 0) {

        auto check_prop = [&](VideoProcAmpProperty prop, const string& name) {
            long min, max, step, default_value, flags, val;
            if (pProcAmp->GetRange(prop, &min, &max, &step, &default_value, &flags) >= 0) {
                if (pProcAmp->Get(prop, &val, &flags) >= 0) {
                    results.push_back({name, val, min, max});
                }
            }
        };

        check_prop(VideoProcAmp_Brightness, "Brightness");
        check_prop(VideoProcAmp_Contrast, "Contrast");
        check_prop(VideoProcAmp_Hue, "Hue");
        check_prop(VideoProcAmp_Saturation, "Saturation");
        check_prop(VideoProcAmp_Sharpness, "Sharpness");
        check_prop(VideoProcAmp_Gamma, "Gamma");
        check_prop(VideoProcAmp_ColorEnable, "ColorEnable");
        check_prop(VideoProcAmp_WhiteBalance, "WhiteBalance");
        check_prop(VideoProcAmp_BacklightCompensation, "BacklightCompensation");
        check_prop(VideoProcAmp_Gain, "Gain");

        pProcAmp->Release();
    }

    IAMCameraControl* pCamControl = nullptr;
    if (data->base_filter_cam->QueryInterface(IID_IAMCameraControl, reinterpret_cast<void**>(&pCamControl)) >= 0) {

        auto check_prop = [&](CameraControlProperty prop, const string& name) {
            long min, max, step, default_value, flags, val;
            if (pCamControl->GetRange(prop, &min, &max, &step, &default_value, &flags) >= 0) {
                if (pCamControl->Get(prop, &val, &flags) >= 0) {
                    results.push_back({name, val, min, max});
                }
            }
        };

        check_prop(CameraControl_Pan, "Pan");
        check_prop(CameraControl_Tilt, "Tilt");
        check_prop(CameraControl_Roll, "Roll");
        check_prop(CameraControl_Zoom, "Zoom");
        check_prop(CameraControl_Exposure, "Exposure");
        check_prop(CameraControl_Iris, "Iris");
        check_prop(CameraControl_Focus, "Focus");

        pCamControl->Release();
    }

    return results;
}

void CameraCapture::setControl(const string& name, int value)
{
    if (!data)
        return;

    auto check_propA = [&](VideoProcAmpProperty prop, const string& prop_name) {
        if (prop_name == name) {
            IAMVideoProcAmp* pProcAmp = nullptr;
            if (data->base_filter_cam->QueryInterface(IID_IAMVideoProcAmp, reinterpret_cast<void**>(&pProcAmp)) >= 0) {
                long min, max, step, default_value, flags;
                if (pProcAmp->GetRange(prop, &min, &max, &step, &default_value, &flags) >= 0) {
                    pProcAmp->Set(prop, value, flags);
                }
                pProcAmp->Release();
            }
        }
    };
    auto check_propB = [&](CameraControlProperty prop, const string& prop_name) {
        if (prop_name == name) {
            IAMCameraControl* pCamControl = nullptr;
            if (data->base_filter_cam->QueryInterface(IID_IAMCameraControl, reinterpret_cast<void**>(&pCamControl)) >= 0) {
                long min, max, step, default_value, flags;
                if (pCamControl->GetRange(prop, &min, &max, &step, &default_value, &flags) >= 0) {
                    pCamControl->Set(prop, value, flags);
                }
                pCamControl->Release();
            }
        }
    };

    check_propA(VideoProcAmp_Brightness, "Brightness");
    check_propA(VideoProcAmp_Contrast, "Contrast");
    check_propA(VideoProcAmp_Hue, "Hue");
    check_propA(VideoProcAmp_Saturation, "Saturation");
    check_propA(VideoProcAmp_Sharpness, "Sharpness");
    check_propA(VideoProcAmp_Gamma, "Gamma");
    check_propA(VideoProcAmp_ColorEnable, "ColorEnable");
    check_propA(VideoProcAmp_WhiteBalance, "WhiteBalance");
    check_propA(VideoProcAmp_BacklightCompensation, "BacklightCompensation");
    check_propA(VideoProcAmp_Gain, "Gain");

    check_propB(CameraControl_Pan, "Pan");
    check_propB(CameraControl_Tilt, "Tilt");
    check_propB(CameraControl_Roll, "Roll");
    check_propB(CameraControl_Zoom, "Zoom");
    check_propB(CameraControl_Exposure, "Exposure");
    check_propB(CameraControl_Iris, "Iris");
    check_propB(CameraControl_Focus, "Focus");
}

CameraCapture::State CameraCapture::getState()
{
    return state;
}

#elif defined(__EMSCRIPTEN__)

class CameraCapture::Data
{
public:
    Data() {}
    ~Data() {
        EM_ASM(
            if (typeof(sp2_video_dom) != 'undefined') {
                sp2_video_dom.srcObject.getTracks().forEach((t) => { t.stop(); });
                sp2_video_dom.srcObject = null;
                sp2_video_dom.remove();
                sp2_canvas_dom.remove();
                sp2_video_dom = undefined;
                sp2_canvas_dom = undefined;
            }
            sp2_video_state = 0;
        );
    }
};

EM_JS(void, sp2_open_video_stream, (int index, int width, int height), {
    if (!navigator.mediaDevices || !navigator.mediaDevices.getUserMedia) {
        //Not supported or not serving with https
        sp2_video_state = 0;
        return;
    }
    sp2_video_state = 1;
    var constraints = { video: {facingMode: "environment"}, audio: false };
    if (index == 1) constraints.video.facingMode = "user";
    if (width > 0 && height > 0) {
        constraints.video.width = width;
        constraints.video.height = height;
    }
    navigator.mediaDevices.getUserMedia(constraints).then((stream) => {
        if (typeof(sp2_video_dom) == 'undefined') {
            sp2_video_dom = document.createElement("video");
            sp2_canvas_dom = document.createElement("canvas");
        }

        sp2_video_dom.srcObject = stream;
        sp2_video_dom.play().then(() => {
            sp2_canvas_dom.width = sp2_video_dom.videoWidth;
            sp2_canvas_dom.height = sp2_video_dom.videoHeight;
            sp2_video_state = 2;
        });
    }).catch(() => {
        sp2_video_state = 0;
    });
});

EM_JS(void, sp2_get_video_image, (uint32_t* image_ptr), {
    var ctx = sp2_canvas_dom.getContext('2d');
    ctx.drawImage(sp2_video_dom, 0, 0);
    var data = ctx.getImageData(0, 0, sp2_canvas_dom.width, sp2_canvas_dom.height);
    var buf = new Uint8ClampedArray(HEAP8.buffer, image_ptr, data.width * data.height * 4);
    buf.set(data.data);
});

CameraCapture::State CameraCapture::init(int index, sp::Vector2i prefered_size)
{
    sp2_open_video_stream(index, prefered_size.x, prefered_size.y);
    return State::Opening;
}

Image CameraCapture::getFrame()
{
    int width = EM_ASM_INT({ return typeof(sp2_video_dom) == 'undefined' ? 0 : sp2_canvas_dom.width;});
    int height = EM_ASM_INT({ return typeof(sp2_video_dom) == 'undefined' ? 0 : sp2_canvas_dom.height;});
    if (width && height) {
        Image img{{width, height}};
        sp2_get_video_image(img.getPtr());
        return img;
    }
    return Image();
}

CameraCapture::State CameraCapture::getState()
{
    int sp2_video_state = EM_ASM_INT({ return sp2_video_state; });
    if (sp2_video_state == 0) state = State::Closed;
    if (sp2_video_state == 1) state = State::Opening;
    if (sp2_video_state == 2) state = State::Streaming;
    return state;
}

std::vector<CameraCapture::Control> CameraCapture::getControls()
{
    return {};
}

void CameraCapture::setControl(const string& name, int value)
{
}

#else

/** Dummy implementation if we do not have an OS specific implementation. */    
class CameraCapture::Data
{
public:
    Data() {}
    ~Data() {}
};

CameraCapture::State CameraCapture::init(int index)
{
    return State::Closed;
}

Image CameraCapture::getFrame()
{
    return Image();
}

std::vector<CameraCapture::Control> CameraCapture::getControls()
{
    return {};
}

void CameraCapture::setControl(const string& name, int value)
{
}

#endif

// Shared CameraCapture implementation
CameraCapture::CameraCapture(int index, sp::Vector2i prefered_size)
: data(nullptr)
{
    data = std::unique_ptr<Data>(new Data());
    state = init(index, prefered_size);

    if (state == State::Closed)
    {
        LOG(Warning, "Failed to open camera:", index);
        data = nullptr;
        state = State::Closed;
    }
}

CameraCapture::~CameraCapture()
{
}

}//namespace io
}//namespace sp
