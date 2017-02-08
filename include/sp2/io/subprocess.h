#ifndef SP2_IO_SUBPROCESS_H
#define SP2_IO_SUBPROCESS_H

#include <sp2/string.h>
#include <SFML/System/NonCopyable.hpp>

namespace sp {
namespace io {

class Subprocess : sf::NonCopyable
{
public:
    class Data;

    Subprocess(std::vector<sp::string> command);

    ~Subprocess();

    int wait();
    int kill();
    
    bool isRunning();
private:
    Data* data;
};

};//!namespace io
};//!namespace sp

#endif//SP2_IO_SUBPROCESS_H
