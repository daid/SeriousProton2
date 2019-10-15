#include <sp2/io/serialport.h>
#include <sp2/logging.h>

#ifdef __WIN32__
    #include <windows.h>
#endif
#if defined(__gnu_linux__) && !defined(__ANDROID__)
    //Including ioctl or termios conflicts with asm/termios.h which we need for TCGETS2. So locally define the ioctl and tcsendbreak functions. Yes, it's dirty, but it works.
    //#include <sys/ioctl.h>
    //#include <termios.h>
    extern "C" {
    extern int ioctl (int __fd, unsigned long int __request, ...) __THROW;
    extern int tcsendbreak (int __fd, int __duration) __THROW;
    }
    #include <asm/termios.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <dirent.h>
#endif
#if defined(__APPLE__) && defined(__MACH__)
    #include <IOKit/serial/ioss.h>
    #include <sys/ioctl.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <termios.h>

    //Define the IOCTL for OSX that allows you to set a custom serial speed, if it's not defined by one of the includes.
    #ifndef IOSSIOSPEED
    #define IOSSIOSPEED _IOW('T', 2, speed_t)
    #endif
#endif

namespace sp {
namespace io {

SerialPort::SerialPort(string name)
{
    std::vector<string> ports = portsByPseudoDriverName(name);
    if (ports.size() > 0)
    {
        LOG(Info, "Selected port:", ports[0], "for pseudo name:", name);
        name = ports[0];
    }
#ifdef __WIN32__
    handle = reinterpret_cast<uintptr_t>(CreateFile(("\\\\.\\" + name).c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
    if (isOpen())
    {
        COMMTIMEOUTS timeouts;
        timeouts.ReadIntervalTimeout = MAXDWORD;
        timeouts.ReadTotalTimeoutMultiplier = 0;
        timeouts.ReadTotalTimeoutConstant = 0;
        timeouts.WriteTotalTimeoutMultiplier = 0;
        timeouts.WriteTotalTimeoutConstant = 0;

        if (!SetCommTimeouts(reinterpret_cast<HANDLE>(handle), &timeouts))
        {
            LOG(Error, "SetCommTimeouts failed!");
        }
    }
#endif
#if (defined(__gnu_linux__) && !defined(__ANDROID__)) || (defined(__APPLE__) && defined(__MACH__))
    if (!name.startswith("/dev/"))
        name = "/dev/" + name;
    handle = open(name.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
#endif
    (void)handle;

    if (!isOpen())
        LOG(Warning, "Failed to open serial port:", name);
}

SerialPort::~SerialPort()
{
    if (!isOpen())
        return;

#ifdef __WIN32__
    CloseHandle(reinterpret_cast<HANDLE>(handle));
    handle = reinterpret_cast<uintptr_t>(INVALID_HANDLE_VALUE);
#endif
#if (defined(__gnu_linux__) && !defined(__ANDROID__)) || (defined(__APPLE__) && defined(__MACH__))
    close(handle);
    handle = 0;
#endif
}

bool SerialPort::isOpen()
{
#ifdef __WIN32__
    return handle != reinterpret_cast<uintptr_t>(INVALID_HANDLE_VALUE);
#endif
#if (defined(__gnu_linux__) && !defined(__ANDROID__)) || (defined(__APPLE__) && defined(__MACH__))
    return handle;
#endif
    return false;
}

void SerialPort::configure(int baudrate, int databits, EParity parity, EStopBits stopbits)
{
    if (!isOpen())
        return;
#ifdef __WIN32__
    FlushFileBuffers(reinterpret_cast<HANDLE>(handle));

    DCB dcb;
    memset(&dcb, 0, sizeof(DCB));
    if (!GetCommState(reinterpret_cast<HANDLE>(handle), &dcb))
    {
        DWORD error;
        ClearCommError(reinterpret_cast<HANDLE>(handle), &error, nullptr);
        LOG(Error, "GetCommState failed!", error);
        return;
    }
    dcb.BaudRate = baudrate;
    dcb.ByteSize = databits;
    switch(parity)
    {
    case NoParity:
        dcb.Parity = NOPARITY;
        break;
    case OddParity:
        dcb.Parity = ODDPARITY;
        break;
    case EvenParity:
        dcb.Parity = EVENPARITY;
        break;
    case MarkParity:
        dcb.Parity = MARKPARITY;
        break;
    }
    switch(stopbits)
    {
    case OneStopBit:
        dcb.StopBits = ONESTOPBIT;
        break;
    case OneAndAHalfStopBit:
        dcb.StopBits = ONE5STOPBITS;
        break;
    case TwoStopbits:
        dcb.StopBits = TWOSTOPBITS;
        break;
    }

    //Do not handle parity errors.
    dcb.fParity = false;

    //Do not discard null chars.
    dcb.fNull = false;

    //Abort on error. Need to call ClearCommError when an error is returned.
    dcb.fAbortOnError = false;

    //Disable all flow control settings, so we can control the DTR and RTS lines manually.
    dcb.fOutxCtsFlow = false;
    dcb.fOutxDsrFlow = false;
    dcb.fDsrSensitivity = false;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
    dcb.fDtrControl = DTR_CONTROL_DISABLE;
    dcb.fTXContinueOnXoff = false;

    if(!SetCommState(reinterpret_cast<HANDLE>(handle), &dcb))
    {
        DWORD error;
        ClearCommError(reinterpret_cast<HANDLE>(handle), &error, nullptr);
        LOG(Error, "SetCommState failed!", error);
    }
#endif
#if defined(__gnu_linux__) && !defined(__ANDROID__)
    fsync(handle);

    struct termios2 tio;
    ioctl(handle, TCGETS2, &tio);

    // Clear handshake, parity, stopbits and size
    tio.c_cflag = CLOCAL;
    tio.c_iflag = 0;
    tio.c_oflag = 0;
    tio.c_lflag = 0;

    // Set the baudrate
    tio.c_cflag &= ~CBAUD;
    tio.c_cflag |= BOTHER;
    tio.c_ispeed = baudrate;
    tio.c_ospeed = baudrate;

    // Enable the receiver
    tio.c_cflag |= CREAD;

    switch (databits)
    {
    case 5:
        tio.c_cflag |= CS5;
        break;
    case 6:
        tio.c_cflag |= CS6;
        break;
    case 7:
        tio.c_cflag |= CS7;
        break;
    default:
    case 8:
        tio.c_cflag |= CS8;
        break;
    }

    switch(parity)
    {
    case NoParity:
        break;
    case OddParity:
        tio.c_cflag |= PARENB | PARODD;
        break;
    case EvenParity:
        tio.c_cflag |= PARENB;
        break;
    case MarkParity:
        tio.c_cflag |= PARENB | PARODD | CMSPAR;
        break;
    }
    switch(stopbits)
    {
    case OneStopBit:
        break;
    case OneAndAHalfStopBit:
        LOG(Warning, "OneAndAHalfStopBit not supported on linux!");
        break;
    case TwoStopbits:
        tio.c_cflag |= CSTOPB;
        break;
    }

    ioctl(handle, TCSETS2, &tio);
#endif
#if defined(__APPLE__) && defined(__MACH__)
    struct termios tio;
    tcgetattr(handle, &tio);

    // Clear handshake, parity, stopbits and size
    tio.c_cflag = CLOCAL;
    tio.c_iflag = 0;
    tio.c_oflag = 0;
    tio.c_lflag = 0;

    // Enable the receiver
    tio.c_cflag |= CREAD;

    switch (databits)
    {
    case 5:
        tio.c_cflag |= CS5;
        break;
    case 6:
        tio.c_cflag |= CS6;
        break;
    case 7:
        tio.c_cflag |= CS7;
        break;
    default:
    case 8:
        tio.c_cflag |= CS8;
        break;
    }

    switch(parity)
    {
    case NoParity:
        tio.c_cflag &= (tcflag_t) ~(PARENB | PARODD);
        break;
    case OddParity:
        tio.c_cflag |= PARENB | PARODD;
        break;
    case EvenParity:
        tio.c_cflag |= PARENB;
        break;
    case MarkParity:
        tio.c_cflag |= PARENB | PARODD | PARMRK;
        break;
    }

    switch(stopbits)
    {
    case OneStopBit:
        break;
    case OneAndAHalfStopBit:
        LOG(Warning, "OneAndAHalfStopBit not supported on posix!");
        break;
    case TwoStopbits:
        tio.c_cflag |= CSTOPB;
        break;
    }

    tcsetattr(handle, TCSANOW, &tio);

    // setting nonstandard baud rate
    speed_t speed = baudrate;
    if (ioctl (handle, IOSSIOSPEED, &speed, 1) < 0) {
        LOG(Error, "setting baud rate failed. errno:", errno);
    }

#endif
}

void SerialPort::send(void* data, int data_size)
{
    if (!isOpen())
        return;
#ifdef __WIN32__
    while(data_size > 0)
    {
        DWORD written = 0;
        if (!WriteFile(reinterpret_cast<HANDLE>(handle), data, data_size, &written, NULL))
        {
            COMSTAT comStat;
            DWORD   dwErrors;
            ClearCommError(reinterpret_cast<HANDLE>(handle), &dwErrors, &comStat);
            return;
        }
        data = static_cast<char*>(data) + written;
        data_size -= written;
    }
#endif
#if (defined(__gnu_linux__) && !defined(__ANDROID__)) || (defined(__APPLE__) && defined(__MACH__))
    while(data_size > 0)
    {
        int written = write(handle, data, data_size);
        if (written < 1)
            return;
        data = static_cast<char*>(data) + written;
        data_size -= written;
    }
#endif
}

int SerialPort::recv(void* data, int data_size)
{
    if (!isOpen())
        return 0;

#ifdef __WIN32__
    DWORD read_size = 0;
    if (!ReadFile(reinterpret_cast<HANDLE>(handle), data, data_size, &read_size, NULL))
    {
        COMSTAT comStat;
        DWORD   dwErrors;
        ClearCommError(reinterpret_cast<HANDLE>(handle), &dwErrors, &comStat);
        return 0;
    }
    return read_size;
#endif
#if (defined(__gnu_linux__) && !defined(__ANDROID__)) || (defined(__APPLE__) && defined(__MACH__))
    int bytes_read = read(handle, data, data_size);
    if (bytes_read > 0)
        return bytes_read;
    return 0;
#endif
    return 0;
}

void SerialPort::setDTR()
{
    if (!isOpen())
        return;
#ifdef __WIN32__
    EscapeCommFunction(reinterpret_cast<HANDLE>(handle), SETDTR);
#endif
#if defined(__gnu_linux__) && !defined(__ANDROID__)
    int bit = TIOCM_DTR;
    ioctl(handle, TIOCMBIS, &bit);
#endif
#if defined(__APPLE__) && defined(__MACH__)
    ioctl(handle, TIOCSDTR);
#endif
}

void SerialPort::clearDTR()
{
    if (!isOpen())
        return;
#ifdef __WIN32__
    EscapeCommFunction(reinterpret_cast<HANDLE>(handle), CLRDTR);
#endif
#if defined(__gnu_linux__) && !defined(__ANDROID__)
    int bit = TIOCM_DTR;
    ioctl(handle, TIOCMBIC, &bit);
#endif
#if defined(__APPLE__) && defined(__MACH__)
    ioctl(handle, TIOCCDTR);
#endif
}

void SerialPort::setRTS()
{
    if (!isOpen())
        return;
#ifdef __WIN32__
    EscapeCommFunction(reinterpret_cast<HANDLE>(handle), SETRTS);
#endif
#if defined(__gnu_linux__) && !defined(__ANDROID__)
    int bit = TIOCM_RTS;
    ioctl(handle, TIOCMBIS, &bit);
#endif
#if defined(__APPLE__) && defined(__MACH__)
    ioctl(handle, TIOCM_RTS);
#endif
}

void SerialPort::clearRTS()
{
    if (!isOpen())
        return;
#ifdef __WIN32__
    EscapeCommFunction(reinterpret_cast<HANDLE>(handle), CLRRTS);
#endif
#if defined(__gnu_linux__) && !defined(__ANDROID__)
    int bit = TIOCM_RTS;
    ioctl(handle, TIOCMBIC, &bit);
#endif
#if defined(__APPLE__) && defined(__MACH__)
    ioctl(handle, TIOCM_RTS);
#endif
}

void SerialPort::sendBreak()
{
#ifdef __WIN32__
    SetCommBreak(reinterpret_cast<HANDLE>(handle));
    Sleep(1);
    ClearCommBreak(reinterpret_cast<HANDLE>(handle));
#endif
#if (defined(__gnu_linux__) && !defined(__ANDROID__)) || (defined(__APPLE__) && defined(__MACH__))
    tcsendbreak(handle, 0);
#endif
}

std::vector<string> SerialPort::getAvailablePorts()
{
    std::vector<string> names;
#ifdef __WIN32__
    HKEY key;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_READ | KEY_QUERY_VALUE, &key) == ERROR_SUCCESS)
    {
        char value[2048];
        unsigned long value_size = sizeof(value);
        unsigned char data[2048];
        unsigned long data_size = sizeof(data);
        int index = 0;

        while(RegEnumValue(key, index, value, &value_size, NULL, NULL, data, &data_size) == ERROR_SUCCESS)
        {
            names.push_back(string(reinterpret_cast<char*>(data)));

            index++;
            value_size = sizeof(value);
            data_size = sizeof(value);
        }
        RegCloseKey(key);
    }else{
        LOG(Error, "Failed to open registry key for serial port list.");
    }
#endif
#if defined(__gnu_linux__) && !defined(__ANDROID__)
    DIR* dir = opendir("/dev/");
    if (dir)
    {
        struct dirent *entry;
        while ((entry = readdir(dir)) != nullptr)
        {
            string filename = entry->d_name;
            if (!filename.startswith("tty"))
                continue;
            if (filename.startswith("ttyACM"))
                names.push_back(filename);
            if (filename.startswith("ttyUSB"))
                names.push_back(filename);
        }
        closedir(dir);
    }
#endif
    return names;
}

string SerialPort::getPseudoDriverName(string port)
{
#ifdef __WIN32__
    string ret;
    HKEY key;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_READ | KEY_QUERY_VALUE, &key) == ERROR_SUCCESS)
    {
        char value[2048];
        unsigned long value_size = sizeof(value);
        unsigned char data[2048];
        unsigned long data_size = sizeof(data);
        int index = 0;

        while(RegEnumValue(key, index, value, &value_size, NULL, NULL, data, &data_size) == ERROR_SUCCESS)
        {
            if (string(reinterpret_cast<char*>(data)) == port)
            {
                //Replace numbers by underscores so matching drivers is easier. As these device names are numbered.
                for(unsigned int n=0; n<value_size; n++)
                    if(value[n] >= '0' && value[n] <= '9')
                        value[n] = '@';

                ret = string(value);
            }
            index++;
            value_size = sizeof(value);
            data_size = sizeof(value);
        }
        RegCloseKey(key);
    }else{
        LOG(Error, "Failed to open registry key for serial port list.");
    }
    return ret;
#endif
#if defined(__gnu_linux__) && !defined(__ANDROID__)
    FILE* f = fopen(("/sys/class/tty/" + port + "/device/modalias").c_str(), "rt");
    if (!f)
        return "";
    char buffer[128];
    buffer[127] = '\0';
    if (!fgets(buffer, 127, f))
    buffer[0] = '\0';
    fclose(f);
    return string(buffer);
#endif
    for(char& c : port)
        if (c >= '0' && c <= '9')
            c = '@';
    return port;
}

std::vector<string> SerialPort::portsByPseudoDriverName(string driver_name)
{
    std::vector<string> driver_names = driver_name.split(";");
    std::vector<string> names;
    for(string driver : driver_names)
    {
        for(string port : getAvailablePorts())
        {
            if (getPseudoDriverName(port) == driver)
                names.push_back(port);
        }
    }
    return names;
}

}//namespace io
}//namespace sp
