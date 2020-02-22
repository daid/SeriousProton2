#ifndef SP2_STRINGUTIL_CONVERT_H
#define SP2_STRINGUTIL_CONVERT_H

#include <sp2/string.h>
#include <sp2/alignment.h>
#include <sp2/graphics/color.h>
#include <sp2/math/vector.h>


namespace sp {
namespace stringutil {
namespace convert {

/* Convert this string to a number */
static inline float toFloat(const string& s) { return atof(s.c_str()); }
static inline int toInt(const string& s, int bits_per_digit=10) { return strtol(s.c_str(), nullptr, bits_per_digit); }
/*
    Convert any string value that might be a true value to boolean true
    Can be "true" "yes" "ok" or any number value that is not zero.
*/
static inline bool toBool(const string& s) { if (s.lower() == "true" || s.lower() == "yes" || s.lower() == "ok") return true; return toInt(s) != 0; }

/*
    Convert a string in the format "x[, y]*" into an std::vector of ints.
    The result has minimal 1 value, even if the string cannot be converted to a int, the value 0 will be in the resulting vector.
 */
std::vector<int> toIntArray(const string& s);

/*
    Convert a string in the format "x[, y]*" into an std::vector of floats.
    The result has minimal 1 value, even if the string cannot be converted to a float, the value 0 will be in the resulting vector.
 */
std::vector<float> toFloatArray(const string& s);
/*
    Convert a string to a 2d vector value.
    Will try to convert "x, y" into a vector. If the y value is missing (only a single number) then the x value is copied into the y value as well.
*/
sp::Vector2d toVector2d(const string& s);
sp::Vector2f toVector2f(const string& s);
sp::Vector2i toVector2i(const string& s);
/*
    Convert a string to a 3d vector value.
    Will try to convert "x, y" into a vector.
    If the y value is missing (only a single number) then the x value is copied into the y and z value as well.
    If the z value is missing (only a single number) then the x value is copied into the z value as well.
*/
sp::Vector3d toVector3d(const string& s);
sp::Vector3f toVector3f(const string& s);
sp::Vector3i toVector3i(const string& s);

Alignment toAlignment(const string& s);

Color toColor(const string& s);

}//namespace convert
}//namespace stringutil
}//namespace sp

#endif//SP2_STRINGUTIL_CONVERT_H
