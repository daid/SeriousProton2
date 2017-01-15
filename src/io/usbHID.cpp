#include <sp2/io/usbHID.h>

#include <sp2/logging.h>

#ifdef __WIN32__
#include <windows.h>
#include <setupapi.h>
extern "C" {
#include "hidsdi.h"
}
#else
#include <string.h>
#include <usb.h>
#endif


namespace sp {
namespace io {

UsbHID::UsbHID(int vendor_id, int product_id)
{
    device_handle = nullptr;
#ifdef __WIN32__
    GUID hid_guid;
    HDEVINFO device_info_list;
    SP_DEVICE_INTERFACE_DATA device_info;
    int i = 0;

    HidD_GetHidGuid(&hid_guid);
    device_info_list = SetupDiGetClassDevs(&hid_guid, NULL, NULL, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
    device_info.cbSize = sizeof(device_info);
    while(device_handle == nullptr)
    {
        if(!SetupDiEnumDeviceInterfaces(device_info_list, 0, &hid_guid, i, &device_info))
            break;
        DWORD size;
        SP_DEVICE_INTERFACE_DETAIL_DATA* device_details = nullptr;
        //Get the size of the details.
        SetupDiGetDeviceInterfaceDetail(device_info_list, &device_info, nullptr, 0, &size, nullptr);
        device_details = (SP_DEVICE_INTERFACE_DETAIL_DATA*)calloc(size, 1);
        device_details->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
        SetupDiGetDeviceInterfaceDetail(device_info_list, &device_info, device_details, size, &size, NULL);
        
        HANDLE handle = CreateFile(device_details->DevicePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
        
        if (handle != INVALID_HANDLE_VALUE)
        {
            HIDD_ATTRIBUTES device_attributes;
            device_attributes.Size = sizeof(HIDD_ATTRIBUTES);
            HidD_GetAttributes(handle, &device_attributes);
            
            LOG(Debug, "USB VID:", sp::string::hex(device_attributes.VendorID), "PID:", sp::string::hex(device_attributes.ProductID));
            //Filter on Vendor and Product ID. Note that filtering on string names has no use.
            //As windows caches these values from the first product that it encounters.
            //Making those values pretty much useless.
            if(device_attributes.VendorID == vendor_id && device_attributes.ProductID == product_id)
            {
                *(HANDLE*)&device_handle = handle;
            }else{
                CloseHandle(handle);
            }
        }
        
        free(device_details);
        i++;
    }

    SetupDiDestroyDeviceInfoList(device_info_list);
#else
    static bool lib_usb_init_done = false;
    if (!lib_usb_init_done)
    {
        usb_init();
        lib_usb_init_done = true;
    }
    
    usb_find_busses();
    usb_find_devices();
    for(struct usb_bus* bus=usb_get_busses(); bus; bus=bus->next)
    {
        for(struct usb_device* dev=bus->devices; dev; dev=dev->next)
        {
            if(dev->descriptor.idVendor == vendor_id && dev->descriptor.idProduct == product_id)
            {
                handle = usb_open(dev);
                if (handle)
                    return;
            }
         }
    }
#endif
}

UsbHID::~UsbHID()
{
    if (device_handle)
    {
#ifdef __WIN32__
        CloseHandle((HANDLE)device_handle);
#else
        usb_close((libusb_device*)handle);
#endif
        device_handle = nullptr;
    }
}

bool UsbHID::isOpen()
{
    return device_handle != nullptr;
}

bool UsbHID::setReport(std::vector<uint8_t> buffer)
{
    if (!device_handle)
        return false;

#ifdef __WIN32__
    if (HidD_SetFeature((HANDLE)device_handle, buffer.data(), buffer.size()))
        return true;
    LOG(Debug, "HidD_SetFeature failed:", GetLastError());
    return false;
#else
    if (usb_control_msg(device_handle, USB_TYPE_CLASS | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, USBRQ_HID_SET_REPORT, USB_HID_REPORT_TYPE_FEATURE << 8 | (buffer[0] & 0xff), 0, buffer.data(), buffer.size(), 5000) != buffer.size())
        return false;
    return true;
#endif
}

/* ------------------------------------------------------------------------ */

std::vector<uint8_t> UsbHID::getReport(int report_number, int report_size)
{
    std::vector<uint8_t> result;
    if (!device_handle)
        return result;
    
    char buffer[report_size + 1];
    buffer[0] = report_number;
#ifdef __WIN32__
    if (HidD_GetFeature((HANDLE)device_handle, buffer, report_size + 1))
    {
        for(int n=0; n<report_size + 1; n++)
            result.push_back(buffer[n]);
    }else{
        LOG(Debug, "HidD_GetFeature failed:", GetLastError());
    }
#else
    int receive_size = usb_control_msg(device_handle, USB_TYPE_CLASS | USB_RECIP_DEVICE | USB_ENDPOINT_IN, USBRQ_HID_GET_REPORT, USB_HID_REPORT_TYPE_FEATURE << 8 | report_number, 0, buffer, report_size + 1, 5000);
    for(int n=0; n<receive_size; n++)
        result.push_back(buffer[n]);
#endif
    return result;
}

};//!namespace io
};//!namespace sp
