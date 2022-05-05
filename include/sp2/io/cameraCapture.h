#ifndef SP2_IO_CAMERA_CAPTURE_H
#define SP2_IO_CAMERA_CAPTURE_H

#include <sp2/graphics/image.h>

namespace sp {
namespace io {

class CameraCapture : sp::NonCopyable
{
public:
    CameraCapture(int index);
    ~CameraCapture();
    
    bool isOpen();
    
    //Get a new frame from the camera stream.
    //Note: On windows this always returns the latest image,
    //      while on linux it might return a zero sized image when there is no new frame available.
    //Note: On web builds, this returns empty images until the user accepted that the application can
    //      use the camera. There is no way to detect a deny or no camera yet.
    Image getFrame();
private:
    bool init(int index);

    class Data;
    std::unique_ptr<Data> data;
    
    sp::Vector2i size;
};

}//namespace io
}//namespace sp

#endif//SP2_IO_CAMERA_CAPTURE_H
