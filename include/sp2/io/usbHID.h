#ifndef SP2_IO_USB_HID_H
#define SP2_IO_USB_HID_H

#include <sp2/string.h>
#include <cstdint>

namespace sp {
namespace io {

//Class to talk to HID devices, by using vendor specific reports.
class UsbHID
{
public:
    UsbHID(int vendor_id, int product_id);
    ~UsbHID();

    bool isOpen();

    bool setReport(std::vector<uint8_t> buffer);
    std::vector<uint8_t> getReport(int report_number, int report_size);
private:
    void* device_handle;
};

};//!namespace io
};//!namespace sp

#endif//SERIAL_DRIVER_H
