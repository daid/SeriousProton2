#ifndef SP2_IO_SUBPROCESS_H
#define SP2_IO_SUBPROCESS_H

#include <sp2/string.h>
#include <sp2/nonCopyable.h>

namespace sp {
namespace io {

class Subprocess : NonCopyable
{
public:
    class Data;

    Subprocess(std::vector<string> command, const string& working_directory="");

    ~Subprocess();

    int wait();
    int kill(bool forcefuly=false);
    
    bool isRunning();
private:
    Data* data;
};

}//namespace io
}//namespace sp

#endif//SP2_IO_SUBPROCESS_H
