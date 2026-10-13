#ifndef SP2_GRAPHICS_IMAGE_RESIZE_H
#define SP2_GRAPHICS_IMAGE_RESIZE_H

#include <sp2/graphics/image.h>


namespace sp::image {

template<typename FMT> ImageBase<FMT> resize(const ImageBase<FMT>& input, Vector2i new_size) {
    ImageBase<FMT> output(new_size);
    auto src = input.getPtr();
    auto dst = output.getPtr();
    int src_y = 0;
    for(int y=0; y<output.getSize().y; y++) {
        int row_count = ((y + 1) * input.getSize().y / new_size.y) - src_y;
        src_y += row_count;
        int src_x = 0;
        for(int x=0; x<output.getSize().x; x++) {
            unsigned int total = 0;
            int col_count = ((x + 1) * input.getSize().x / new_size.x) - src_x;
            src_x += col_count;
            for(int col=0; col<col_count; col++) {
                for(int row=0; row<row_count*input.getSize().x; row+=input.getSize().x)
                    total += src[row];
                src++;
            }
            *dst++ = total / col_count / row_count;
        }
        src += (row_count - 1) * input.getSize().x;
    }
    return output;
}

}//namespace sp::image


#endif//SP2_GRAPHICS_IMAGE_RESIZE_H
