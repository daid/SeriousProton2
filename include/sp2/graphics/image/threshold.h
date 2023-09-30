#ifndef SP2_GRAPHICS_IMAGE_THRESHOLD_H
#define SP2_GRAPHICS_IMAGE_THRESHOLD_H

#include <sp2/graphics/image.h>


namespace sp::image {

static inline sp::Image8 threshold(const sp::Image8& input, int threshold_value, int neg=0, int pos=255) {
    sp::Image8 output{input.getSize()};
    auto ptr_i = input.getPtr();
    auto ptr_o = output.getPtr();
    for(auto n=0; n<input.getSize().x*input.getSize().y;n++) {
        if (*ptr_i >= threshold_value)
            *ptr_o = pos;
        else
            *ptr_o = neg;
        ptr_i++;
        ptr_o++;
    }
    return output;
}

static inline sp::Image8 adaptiveThreshold(const sp::Image8& input, int blocksize, int offset, int neg=0, int pos=255) {
    auto output = sp::image::gaussianBlur(input, blocksize);
    auto ptr_i = input.getPtr();
    auto ptr_o = output.getPtr();
    for(auto n=0; n<input.getSize().x*input.getSize().y;n++) {
        if (*ptr_i > *ptr_o + offset)
            *ptr_o = pos;
        else
            *ptr_o = neg;
        ptr_i++;
        ptr_o++;
    }
    return output;
}

}//namespace sp::image


#endif//SP2_GRAPHICS_IMAGE_THRESHOLD_H
