#ifndef SP2_STRINGUTIL_UTF8_H
#define SP2_STRINGUTIL_UTF8_H

#include <sp2/string.h>

namespace sp {
namespace stringutil {
namespace utf8 {

int decodeSingle(const char* str, int* consumed_characters);

};//namespace utf8
};//namespace stringutil
};//namespace sp

#endif//SP2_STRINGUTIL_UTF8_H
