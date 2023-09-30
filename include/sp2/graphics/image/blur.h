#ifndef SP2_GRAPHICS_IMAGE_BLUR_H
#define SP2_GRAPHICS_IMAGE_BLUR_H

#include <sp2/graphics/image.h>


namespace sp::image {

namespace detail {
    template<typename T> struct PixelInfo;
    template<> struct PixelInfo<uint8_t> { double total = 0;
        void mulacc(double mul, uint8_t in) { total += mul * in; }
        uint8_t result() { return total; }
    };
    template<> struct PixelInfo<uint32_t> { double r = 0, g = 0, b = 0;
        void mulacc(double mul, uint32_t in) { r += mul * (in & 0xFF); g += mul * ((in >> 8) & 0xFF); b += mul * ((in >> 16) & 0xFF); }
        uint32_t result() { return int(r) | int(g) << 8 | int(b) << 16 | 0xFF000000; }
    };
}

template<typename T> ImageBase<T> gaussianBlur(const ImageBase<T>& input, int amount)
{
    ImageBase<T> intermediate{input.getSize()};
    ImageBase<T> output{input.getSize()};
    auto ksize = amount*2+1;
    double kernel[ksize];
    auto size = input.getSize();
    if (amount == 0) {
        kernel[0] = 1.0;
    } else if (amount == 1) {
        kernel[0] = 0.25;
        kernel[1] = 0.5;
        kernel[2] = 0.25;
    } else if (amount == 2) {
        kernel[0] = 0.0625;
        kernel[1] = 0.25;
        kernel[2] = 0.375;
        kernel[3] = 0.25;
        kernel[4] = 0.0625;
    } else if (amount == 3) {
        kernel[0] = 0.03125;
        kernel[1] = 0.109375;
        kernel[2] = 0.21875;
        kernel[3] = 0.28125;
        kernel[4] = 0.21875;
        kernel[5] = 0.109375;
        kernel[6] = 0.03125;
    } else if (amount == 4) {
        kernel[0] = 4.0 / 256.0;
        kernel[1] = 13.0 / 256.0;
        kernel[2] = 30.0 / 256.0;
        kernel[3] = 51.0 / 256.0;
        kernel[4] = 60.0 / 256;
        kernel[5] = 51.0 / 256.0;
        kernel[6] = 30.0 / 256.0;
        kernel[7] = 13.0 / 256.0;
        kernel[8] = 4.0 / 256.0;
    } else {
        double sigma = ((ksize-1)*0.5 - 1)*0.3 + 0.8;
        double total = 0.0;
        for(int n=0; n<ksize; n++) {
            kernel[n] = std::exp(-0.5 * (std::pow((n-amount)/sigma, 2.0)));
            total += kernel[n];
        }
        for(int n=0; n<ksize; n++)
            kernel[n] /= total;
    }

    auto ptr_i = input.getPtr();
    auto ptr_o = intermediate.getPtr();
    for(int y=0; y<amount;y++) {
        ptr_i = input.getPtr();
        for(int x=0; x<size.x; x++) {
            detail::PixelInfo<T> pix;
            for(int n=0; n<ksize;n++)
                pix.mulacc(kernel[n], ptr_i[std::max(0, n-amount+y)*size.x]);
            *ptr_o = pix.result();
            ptr_i++;
            ptr_o++;
        }
    }
    ptr_i = input.getPtr();
    for(int y=amount; y<size.y-1-amount;y++) {
        for(int x=0; x<size.x; x++) {
            detail::PixelInfo<T> pix;
            for(int n=0; n<ksize;n++)
                pix.mulacc(kernel[n], ptr_i[n*size.x]);
            *ptr_o = pix.result();
            ptr_i++;
            ptr_o++;
        }
    }
    for(int y=0; y<amount;y++) {
        ptr_i = input.getPtr() + size.x * (size.y - ksize);
        for(int x=0; x<size.x; x++) {
            detail::PixelInfo<T> pix;
            for(int n=0; n<ksize;n++)
                pix.mulacc(kernel[n], ptr_i[std::min(ksize-1, n+y)*size.x]);
            *ptr_o = pix.result();
            ptr_i++;
            ptr_o++;
        }
    }

    ptr_i = intermediate.getPtr();
    ptr_o = output.getPtr();
    for(int y=0; y<size.y; y++) {
        for(int x=0; x<amount; x++) {
            detail::PixelInfo<T> pix;
            for(int n=0; n<ksize;n++)
                pix.mulacc(kernel[n], ptr_i[std::max(0, n-amount+x)]);
            *ptr_o = pix.result();
            ptr_o++;
        }
        for(int x=amount; x<size.x-amount; x++) {
            detail::PixelInfo<T> pix;
            for(int n=0; n<ksize;n++)
                pix.mulacc(kernel[n], ptr_i[n]);
            *ptr_o = pix.result();
            ptr_i++;
            ptr_o++;
        }
        for(int x=0; x<amount; x++) {
            detail::PixelInfo<T> pix;
            for(int n=0; n<ksize;n++)
                pix.mulacc(kernel[n], ptr_i[std::min(ksize-1, n+x)]);
            *ptr_o = pix.result();
            ptr_o++;
        }
        ptr_i += amount*2;
    }

    return output;
}

}//namespace sp::image


#endif//SP2_GRAPHICS_IMAGE_BLUR_H
