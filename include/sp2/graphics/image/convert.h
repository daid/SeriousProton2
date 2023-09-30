#ifndef SP2_GRAPHICS_IMAGE_CONVERT_H
#define SP2_GRAPHICS_IMAGE_CONVERT_H

#include <sp2/graphics/image.h>


namespace sp::image {

namespace detail {
    template<typename DST, typename SRC> struct PixelConvertInfo;
    template<> struct PixelConvertInfo<uint32_t, uint8_t> { static inline uint8_t conv(uint32_t in) { return ((in & 0xFF) + ((in >> 8) & 0xFF) + ((in >> 16) & 0xFF)) / 3; } };
    template<> struct PixelConvertInfo<uint8_t, uint32_t> { static inline uint32_t conv(uint8_t in) { return 0xFF000000 | in | (in << 8) | (in << 16); } };
}

template<typename DST, typename SRC> ImageBase<DST> convert(const ImageBase<SRC>& input) {
    ImageBase<DST> output(input.getSize());
    auto src = input.getPtr();
    auto dst = output.getPtr();
    for(int n=0; n<input.getSize().x*input.getSize().y; n++)
        *dst++ = detail::PixelConvertInfo<DST, SRC>::conv(*src++);
    return output;
}

}//namespace sp::image


#endif//SP2_GRAPHICS_IMAGE_CONVERT_H
