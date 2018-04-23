#ifndef SP2_EXCEPTION_UNHANDLED_H
#define SP2_EXCEPTION_UNHANDLED_H

#include <sp2/exception/addressInfo.h>
#include <functional>

namespace sp {
namespace exception {

class UnhandledExceptionInfo
{
public:
    std::string base_report;
    std::vector<AddressInfo> stack;
};
typedef std::function<void(const UnhandledExceptionInfo& info)> UnhandledExceptionCallback;
void setUnhandledExceptionCallback(UnhandledExceptionCallback callback);

};//namespace exception
};//namespace sp

#endif//SP2_EXCEPTION_UNHANDLED_H
