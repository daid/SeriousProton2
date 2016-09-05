#ifndef SP2_IO_SERIALPORT_H
#define SP2_IO_SERIALPORT_H

#include <sp2/string.h>
#include <cstdint>

namespace sp {
namespace io {

//Class to interact with serial ports. Abstracts the difference between UNIX and Windows API.
//  And uses some tricks to help identify serial ports.
class SerialPort
{
private:
    uintptr_t handle;

public:
    enum EParity
    {
        NoParity,
        OddParity,
        EvenParity,
        MarkParity
    };
    enum EStopBits
    {
        OneStopBit,
        OneAndAHalfStopBit,
        TwoStopbits
    };

    SerialPort(string name);
    ~SerialPort();
    
    bool isOpen();
    
    void configure(int baudrate, int databits, EParity parity, EStopBits stopbits);
    
    void send(void* data, int data_size);
    int recv(void* data, int data_size);
    
    void setDTR();
    void clearDTR();
    void setRTS();
    void clearRTS();
    void sendBreak();
    
    static std::vector<string> getAvailablePorts();
    static string getPseudoDriverName(string port);
    static std::vector<string> portsByPseudoDriverName(string driver_name);
};

};//!namespace io
};//!namespace sp

#endif//SERIAL_DRIVER_H
