#ifndef SP2_STRINGUTIL_BASE64_H
#define SP2_STRINGUTIL_BASE64_H

#include <sp2/string.h>

namespace sp {
namespace stringutil {
namespace base64 {

string encode(string data);
string decode(string data);

};//namespace base64
};//namespace stringutil
};//namespace sp

#endif//SP2_STRINGUTIL_BASE64_H

