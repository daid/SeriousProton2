#include <sp2/exception/unhandled.h>

namespace sp {
namespace exception {

void setupUnhandledExceptionHandler() __attribute__((weak));
void setupUnhandledExceptionHandler()
{
}

};//!namespace exception
};//!namespace sp
