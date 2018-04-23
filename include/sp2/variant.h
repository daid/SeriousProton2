#ifndef SP2_VARIANT_H
#define SP2_VARIANT_H

#include <sp2/string.h>

namespace sp {

class Variant
{
public:
    Variant() { type = Type::None; }
    Variant(int i) { type = Type::Integer; data.i = i; }
    Variant(double d) { type = Type::Double; data.d = d; }
    Variant(string s) { type = Type::String; str = s; }

    inline bool isNone() { return type == Type::None; }
    inline bool isInteger() { return type == Type::Integer; }
    inline bool isDouble() { return type == Type::Double; }
    inline bool isString() { return type == Type::String; }

    inline int getInteger() { if (isInteger()) return data.i; else if (isDouble()) return data.d; return 0; }
    inline double getDouble() { if (isDouble()) return data.d; else if (isInteger()) return data.i; return 0; }
    inline string getString() { if (isString()) return str; if (isDouble()) return string(data.d); else if (isInteger()) return string(data.i); return ""; }

private:
    enum class Type
    {
        None,
        Integer,
        Double,
        String
    };
    Type type;
    union
    {
        int i;
        double d;
    } data;
    string str;
};

};//namespace sp

#endif//SP2_VARIANT_H
