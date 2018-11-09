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
    
    Image getFrame();
private:
    bool init(int index);

    class Data;
    Data* data;
    
    sp::Vector2i size;
};

};//namespace io
};//namespace sp

#endif//SP2_IO_CAMERA_CAPTURE_H
