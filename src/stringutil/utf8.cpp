#include <sp2/stringutil/utf8.h>
#include <sp2/logging.h>


namespace sp {
namespace stringutil {
namespace utf8 {

int decodeSingle(const char* str, int* consumed_characters)
{
    if (consumed_characters) *consumed_characters = 1;
    if ((str[0] & 0xe0) == 0xc0)
    {
        if ((str[1] & 0xc0) != 0x80)
        {
            LOG(Warning, "Malform utf-8 string.");
            return '?';
        }
        if (consumed_characters) *consumed_characters = 2;
        return ((str[0] & 0x1f) << 6) | (str[1] & 0x3f);
    }
    else if ((str[0] & 0xf0) == 0xe0)
    {
        if ((str[1] & 0xc0) != 0x80 || (str[2] & 0xc0) != 0x80)
        {
            LOG(Warning, "Malform utf-8 string.");
            return '?';
        }
        if (consumed_characters) *consumed_characters = 3;
        return ((str[0] & 0x0f) << 12) | ((str[1] & 0x3f) << 6) | (str[2] & 0x3f);
    }
    else if ((str[0] & 0xf8) == 0xf0)
    {
        if ((str[1] & 0xc0) != 0x80 || (str[2] & 0xc0) != 0x80 || (str[3] & 0xc0) != 0x80)
        {
            LOG(Warning, "Malform utf-8 string.");
            return '?';
        }
        if (consumed_characters) *consumed_characters = 4;
        return ((str[0] & 0x0f) << 18) | ((str[1] & 0x3f) << 12) | ((str[2] & 0x3f) << 6) | (str[3] & 0x3f);
    }
    else if (str[0] & 0x80)
    {
        LOG(Warning, "Malform utf-8 string.");
        return '?';
    }
    return str[0];
}

}//namespace base64
}//namespace stringutil
}//namespace sp

