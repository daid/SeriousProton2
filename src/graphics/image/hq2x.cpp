#include <sp2/graphics/image/hq2x.h>
#include <sp2/logging.h>

namespace sp {
namespace image {

static uint32_t toYUV(uint32_t rgb)
{
    float r = (rgb & 0xff0000) >> 16;
    float g = (rgb & 0x00ff00) >> 8;
    float b = (rgb & 0x0000ff);

    uint32_t y = 0.299 * r + 0.587 * g + 0.114 * b;
    uint32_t u = (-0.169 * r - 0.331 * g + 0.5 * b) + 128;
    uint32_t v = (0.5 * r - 0.419 * g - 0.081 * b) + 128;
    return (y << 16) | (u << 8) | (v);
}

static bool diff(uint32_t color0, uint32_t color1, int32_t trY, int32_t trU, int32_t trV, int32_t trA)
{
    int32_t yuv0 = toYUV(color0);
    int32_t yuv1 = toYUV(color1);
    
    return (abs((yuv0 & 0xff0000) - (yuv1 & 0xff0000)) > trY)
        || (abs((yuv0 & 0x00ff00) - (yuv1 & 0x00ff00)) > trU)
        || (abs((yuv0 & 0x0000ff) - (yuv1 & 0x0000ff)) > trV)
        || (abs((color0 >> 24) - (color1 >> 24)) > trA);
}

static inline uint32_t mix(uint32_t w1, uint32_t w2, uint32_t c1, uint32_t c2)
{
    uint32_t w = 0;
    uint32_t g = 0;
    uint32_t rb = 0;
    uint32_t a = 0;
    
    if (c1 >> 24)
    {
        a += (c1 >> 24) * w1;
        w += w1;
        g += (c1 & 0x00ff00) * w1;
        rb += (c1 & 0xff00ff) * w1;
    }
    if (c2 >> 24)
    {
        a += (c2 >> 24) * w2;
        w += w2;
        g += (c2 & 0x00ff00) * w2;
        rb += (c2 & 0xff00ff) * w2;
    }
    a /= w1 + w2;
    if (w > 0)
    {
        g = (g / w) & 0x00ff00;
        rb = (rb / w) & 0xff00ff;
    }
    return a << 24 | g | rb;
}

static inline uint32_t mix(uint32_t w1, uint32_t w2, uint32_t w3, uint32_t c1, uint32_t c2, uint32_t c3)
{
    uint32_t w = 0;
    uint32_t g = 0;
    uint32_t rb = 0;
    uint32_t a = 0;
    
    if (c1 >> 24)
    {
        a += (c1 >> 24) * w1;
        w += w1;
        g += (c1 & 0x00ff00) * w1;
        rb += (c1 & 0xff00ff) * w1;
    }
    if (c2 >> 24)
    {
        a += (c2 >> 24) * w2;
        w += w2;
        g += (c2 & 0x00ff00) * w2;
        rb += (c2 & 0xff00ff) * w2;
    }
    if (c3 >> 24)
    {
        a += (c3 >> 24) * w3;
        w += w3;
        g += (c3 & 0x00ff00) * w3;
        rb += (c3 & 0xff00ff) * w3;
    }
    a /= w1 + w2 + w3;
    if (w > 0)
    {
        g = (g / w) & 0x00ff00;
        rb = (rb / w) & 0xff00ff;
    }
    return a << 24 | g | rb;
}

static void hq2xProcess(const uint32_t* src, uint32_t* dst, sp::Vector2i src_size, int src_stride, HQ2xConfig config)
{
    uint32_t trY = 48;
    uint32_t trU = 7;
    uint32_t trV = 6;
    uint32_t trA = 0;
    
    trY <<= 16;
    trU <<= 8;
    int dst_stride = src_stride * 2;

    int prevline_offset, nextline_offset;
    uint32_t w[9];

    for(int y = 0; y<src_size.y; y++)
    {
        if (y > 0)
            prevline_offset = -src_stride;
        else if (config.out_of_bounds == HQ2xConfig::OutOfBounds::Wrap)
            prevline_offset = src_stride * (src_size.y - 1);
        else
            prevline_offset = 0;
        if (y < src_size.y - 1)
            nextline_offset = src_stride;
        else if (config.out_of_bounds == HQ2xConfig::OutOfBounds::Wrap)
            nextline_offset = -(src_stride * (src_size.y - 1));
        else
            nextline_offset = 0;

        for(int x = 0; x<src_size.x; x++)
        {
            w[1] = src[prevline_offset];
            w[4] = src[0];
            w[7] = src[nextline_offset];

            if (x > 0)
            {
                w[0] = src[prevline_offset - 1];
                w[3] = src[-1];
                w[6] = src[nextline_offset - 1];
            }
            else if (config.out_of_bounds == HQ2xConfig::OutOfBounds::Wrap)
            {
                w[0] = src[prevline_offset + src_size.x - 1];
                w[3] = src[src_size.x - 1];
                w[6] = src[nextline_offset + src_size.x - 1];
            }
            else if (config.out_of_bounds == HQ2xConfig::OutOfBounds::Clamp)
            {
                w[0] = w[1];
                w[3] = w[4];
                w[6] = w[7];
            }
            else if (config.out_of_bounds == HQ2xConfig::OutOfBounds::Transparent)
            {
                w[0] = 0;
                w[3] = 0;
                w[6] = 0;
            }

            if (x < src_size.x - 1)
            {
                w[2] = src[prevline_offset + 1];
                w[5] = src[1];
                w[8] = src[nextline_offset + 1];
            }
            else if (config.out_of_bounds == HQ2xConfig::OutOfBounds::Wrap)
            {
                w[2] = src[prevline_offset - src_size.x + 1];
                w[5] = src[-src_size.x + 1];
                w[8] = src[nextline_offset - src_size.x + 1];
            }
            else if (config.out_of_bounds == HQ2xConfig::OutOfBounds::Clamp)
            {
                w[2] = w[1];
                w[5] = w[4];
                w[8] = w[7];
            }
            else if (config.out_of_bounds == HQ2xConfig::OutOfBounds::Transparent)
            {
                w[2] = 0;
                w[5] = 0;
                w[8] = 0;
            }

            if (y == 0 && config.out_of_bounds == HQ2xConfig::OutOfBounds::Transparent)
                w[0] = w[1] = w[2] = 0;
            if (y == src_size.y - 1 && config.out_of_bounds == HQ2xConfig::OutOfBounds::Transparent)
                w[6] = w[7] = w[8] = 0;

            int pattern = 0;
            int flag = 1;

            for(int k = 0; k < 9; k++)
            {
                if (k == 4) continue;

                if (w[k] != w[4])
                {
                    if (diff(w[4], w[k], trY, trU, trV, trA))
                        pattern |= flag;
                }
                flag <<= 1;
            }

            switch(pattern)
            {
            case 0:
            case 1:
            case 4:
            case 32:
            case 128:
            case 5:
            case 132:
            case 160:
            case 33:
            case 129:
            case 36:
            case 133:
            case 164:
            case 161:
            case 37:
            case 165:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    break;
                }
            case 2:
            case 34:
            case 130:
            case 162:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[3]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[5]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    break;
                }
            case 16:
            case 17:
            case 48:
            case 49:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[1]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[7]);
                    break;
                }
            case 64:
            case 65:
            case 68:
            case 69:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[3]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[5]);
                    break;
                }
            case 8:
            case 12:
            case 136:
            case 140:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[7]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    break;
                }
            case 3:
            case 35:
            case 131:
            case 163:
                {
                    dst[0] = mix(3, 1, w[4], w[3]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[5]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    break;
                }
            case 6:
            case 38:
            case 134:
            case 166:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[3]);
                    dst[1] = mix(3, 1, w[4], w[5]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    break;
                }
            case 20:
            case 21:
            case 52:
            case 53:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    dst[1] = mix(3, 1, w[4], w[1]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[7]);
                    break;
                }
            case 144:
            case 145:
            case 176:
            case 177:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[1]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[7]);
                    break;
                }
            case 192:
            case 193:
            case 196:
            case 197:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[3]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[5]);
                    break;
                }
            case 96:
            case 97:
            case 100:
            case 101:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    dst[dst_stride] = mix(3, 1, w[4], w[3]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[5]);
                    break;
                }
            case 40:
            case 44:
            case 168:
            case 172:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    dst[dst_stride] = mix(3, 1, w[4], w[7]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    break;
                }
            case 9:
            case 13:
            case 137:
            case 141:
                {
                    dst[0] = mix(3, 1, w[4], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[7]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    break;
                }
            case 18:
            case 50:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[3]);
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = mix(3, 1, w[4], w[2]);
                    }
                    else
                    {
                        dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[7]);
                    break;
                }
            case 80:
            case 81:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[1]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[3]);
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = mix(3, 1, w[4], w[8]);
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 72:
            case 76:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = mix(3, 1, w[4], w[6]);
                    }
                    else
                    {
                        dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    }
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[5]);
                    break;
                }
            case 10:
            case 138:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                    }
                    else
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    }
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[5]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[7]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    break;
                }
            case 66:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[3]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[5]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[3]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[5]);
                    break;
                }
            case 24:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[1]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[7]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[7]);
                    break;
                }
            case 7:
            case 39:
            case 135:
                {
                    dst[0] = mix(3, 1, w[4], w[3]);
                    dst[1] = mix(3, 1, w[4], w[5]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    break;
                }
            case 148:
            case 149:
            case 180:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    dst[1] = mix(3, 1, w[4], w[1]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[7]);
                    break;
                }
            case 224:
            case 228:
            case 225:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    dst[dst_stride] = mix(3, 1, w[4], w[3]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[5]);
                    break;
                }
            case 41:
            case 169:
            case 45:
                {
                    dst[0] = mix(3, 1, w[4], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    dst[dst_stride] = mix(3, 1, w[4], w[7]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    break;
                }
            case 22:
            case 54:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[3]);
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = w[4];
                    }
                    else
                    {
                        dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[7]);
                    break;
                }
            case 208:
            case 209:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[1]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[3]);
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 104:
            case 108:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = w[4];
                    }
                    else
                    {
                        dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    }
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[5]);
                    break;
                }
            case 11:
            case 139:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    }
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[5]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[7]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    break;
                }
            case 19:
            case 51:
                {
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[0] = mix(3, 1, w[4], w[3]);
                        dst[1] = mix(3, 1, w[4], w[2]);
                    }
                    else
                    {
                        dst[0] = mix(5, 2, 1, w[4], w[1], w[3]);
                        dst[1] = mix(2, 3, 3, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[7]);
                    break;
                }
            case 146:
            case 178:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[3]);
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride + 1] = mix(3, 1, w[4], w[7]);
                    }
                    else
                    {
                        dst[1] = mix(2, 3, 3, w[4], w[1], w[5]);
                        dst[dst_stride + 1] = mix(5, 2, 1, w[4], w[5], w[7]);
                    }
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    break;
                }
            case 84:
            case 85:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride + 1] = mix(3, 1, w[4], w[8]);
                    }
                    else
                    {
                        dst[1] = mix(5, 2, 1, w[4], w[5], w[1]);
                        dst[dst_stride + 1] = mix(2, 3, 3, w[4], w[5], w[7]);
                    }
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[3]);
                    break;
                }
            case 112:
            case 113:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[1]);
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = mix(3, 1, w[4], w[8]);
                    }
                    else
                    {
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[7], w[3]);
                        dst[dst_stride + 1] = mix(2, 3, 3, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 200:
            case 204:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + 1] = mix(3, 1, w[4], w[5]);
                    }
                    else
                    {
                        dst[dst_stride] = mix(2, 3, 3, w[4], w[7], w[3]);
                        dst[dst_stride + 1] = mix(5, 2, 1, w[4], w[7], w[5]);
                    }
                    break;
                }
            case 73:
            case 77:
                {
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[0] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride] = mix(3, 1, w[4], w[6]);
                    }
                    else
                    {
                        dst[0] = mix(5, 2, 1, w[4], w[3], w[1]);
                        dst[dst_stride] = mix(2, 3, 3, w[4], w[7], w[3]);
                    }
                    dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[5]);
                    break;
                }
            case 42:
            case 170:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride] = mix(3, 1, w[4], w[7]);
                    }
                    else
                    {
                        dst[0] = mix(2, 3, 3, w[4], w[3], w[1]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[7]);
                    }
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[5]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    break;
                }
            case 14:
            case 142:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[5]);
                    }
                    else
                    {
                        dst[0] = mix(2, 3, 3, w[4], w[3], w[1]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[7]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    break;
                }
            case 67:
                {
                    dst[0] = mix(3, 1, w[4], w[3]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[5]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[3]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[5]);
                    break;
                }
            case 70:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[3]);
                    dst[1] = mix(3, 1, w[4], w[5]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[3]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[5]);
                    break;
                }
            case 28:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[1]);
                    dst[1] = mix(3, 1, w[4], w[1]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[7]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[7]);
                    break;
                }
            case 152:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[1]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[7]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[7]);
                    break;
                }
            case 194:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[3]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[5]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[3]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[5]);
                    break;
                }
            case 98:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[3]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[5]);
                    dst[dst_stride] = mix(3, 1, w[4], w[3]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[5]);
                    break;
                }
            case 56:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[1]);
                    dst[dst_stride] = mix(3, 1, w[4], w[7]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[7]);
                    break;
                }
            case 25:
                {
                    dst[0] = mix(3, 1, w[4], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[1]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[7]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[7]);
                    break;
                }
            case 26:
            case 31:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    }
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = w[4];
                    }
                    else
                    {
                        dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[7]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[7]);
                    break;
                }
            case 82:
            case 214:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[3]);
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = w[4];
                    }
                    else
                    {
                        dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[3]);
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 88:
            case 248:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[1]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = w[4];
                    }
                    else
                    {
                        dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    }
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 74:
            case 107:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    }
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[5]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = w[4];
                    }
                    else
                    {
                        dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    }
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[5]);
                    break;
                }
            case 27:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    }
                    dst[1] = mix(3, 1, w[4], w[2]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[7]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[7]);
                    break;
                }
            case 86:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[3]);
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = w[4];
                    }
                    else
                    {
                        dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[3]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[8]);
                    break;
                }
            case 216:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[1]);
                    dst[dst_stride] = mix(3, 1, w[4], w[6]);
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 106:
                {
                    dst[0] = mix(3, 1, w[4], w[0]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[5]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = w[4];
                    }
                    else
                    {
                        dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    }
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[5]);
                    break;
                }
            case 30:
                {
                    dst[0] = mix(3, 1, w[4], w[0]);
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = w[4];
                    }
                    else
                    {
                        dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[7]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[7]);
                    break;
                }
            case 210:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[3]);
                    dst[1] = mix(3, 1, w[4], w[2]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[3]);
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 120:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[1]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = w[4];
                    }
                    else
                    {
                        dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    }
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[8]);
                    break;
                }
            case 75:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    }
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[5]);
                    dst[dst_stride] = mix(3, 1, w[4], w[6]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[5]);
                    break;
                }
            case 29:
                {
                    dst[0] = mix(3, 1, w[4], w[1]);
                    dst[1] = mix(3, 1, w[4], w[1]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[7]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[7]);
                    break;
                }
            case 198:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[3]);
                    dst[1] = mix(3, 1, w[4], w[5]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[3]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[5]);
                    break;
                }
            case 184:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[1]);
                    dst[dst_stride] = mix(3, 1, w[4], w[7]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[7]);
                    break;
                }
            case 99:
                {
                    dst[0] = mix(3, 1, w[4], w[3]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[5]);
                    dst[dst_stride] = mix(3, 1, w[4], w[3]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[5]);
                    break;
                }
            case 57:
                {
                    dst[0] = mix(3, 1, w[4], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[1]);
                    dst[dst_stride] = mix(3, 1, w[4], w[7]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[7]);
                    break;
                }
            case 71:
                {
                    dst[0] = mix(3, 1, w[4], w[3]);
                    dst[1] = mix(3, 1, w[4], w[5]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[3]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[5]);
                    break;
                }
            case 156:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[1]);
                    dst[1] = mix(3, 1, w[4], w[1]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[7]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[7]);
                    break;
                }
            case 226:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[3]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[5]);
                    dst[dst_stride] = mix(3, 1, w[4], w[3]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[5]);
                    break;
                }
            case 60:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[1]);
                    dst[1] = mix(3, 1, w[4], w[1]);
                    dst[dst_stride] = mix(3, 1, w[4], w[7]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[7]);
                    break;
                }
            case 195:
                {
                    dst[0] = mix(3, 1, w[4], w[3]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[5]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[3]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[5]);
                    break;
                }
            case 102:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[3]);
                    dst[1] = mix(3, 1, w[4], w[5]);
                    dst[dst_stride] = mix(3, 1, w[4], w[3]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[5]);
                    break;
                }
            case 153:
                {
                    dst[0] = mix(3, 1, w[4], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[1]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[7]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[7]);
                    break;
                }
            case 58:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                    }
                    else
                    {
                        dst[0] = mix(6, 1, 1, w[4], w[3], w[1]);
                    }
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = mix(3, 1, w[4], w[2]);
                    }
                    else
                    {
                        dst[1] = mix(6, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(3, 1, w[4], w[7]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[7]);
                    break;
                }
            case 83:
                {
                    dst[0] = mix(3, 1, w[4], w[3]);
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = mix(3, 1, w[4], w[2]);
                    }
                    else
                    {
                        dst[1] = mix(6, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[3]);
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = mix(3, 1, w[4], w[8]);
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 92:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[1]);
                    dst[1] = mix(3, 1, w[4], w[1]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = mix(3, 1, w[4], w[6]);
                    }
                    else
                    {
                        dst[dst_stride] = mix(6, 1, 1, w[4], w[7], w[3]);
                    }
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = mix(3, 1, w[4], w[8]);
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 202:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                    }
                    else
                    {
                        dst[0] = mix(6, 1, 1, w[4], w[3], w[1]);
                    }
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[5]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = mix(3, 1, w[4], w[6]);
                    }
                    else
                    {
                        dst[dst_stride] = mix(6, 1, 1, w[4], w[7], w[3]);
                    }
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[5]);
                    break;
                }
            case 78:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                    }
                    else
                    {
                        dst[0] = mix(6, 1, 1, w[4], w[3], w[1]);
                    }
                    dst[1] = mix(3, 1, w[4], w[5]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = mix(3, 1, w[4], w[6]);
                    }
                    else
                    {
                        dst[dst_stride] = mix(6, 1, 1, w[4], w[7], w[3]);
                    }
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[5]);
                    break;
                }
            case 154:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                    }
                    else
                    {
                        dst[0] = mix(6, 1, 1, w[4], w[3], w[1]);
                    }
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = mix(3, 1, w[4], w[2]);
                    }
                    else
                    {
                        dst[1] = mix(6, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[7]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[7]);
                    break;
                }
            case 114:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[3]);
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = mix(3, 1, w[4], w[2]);
                    }
                    else
                    {
                        dst[1] = mix(6, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(3, 1, w[4], w[3]);
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = mix(3, 1, w[4], w[8]);
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 89:
                {
                    dst[0] = mix(3, 1, w[4], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[1]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = mix(3, 1, w[4], w[6]);
                    }
                    else
                    {
                        dst[dst_stride] = mix(6, 1, 1, w[4], w[7], w[3]);
                    }
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = mix(3, 1, w[4], w[8]);
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 90:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                    }
                    else
                    {
                        dst[0] = mix(6, 1, 1, w[4], w[3], w[1]);
                    }
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = mix(3, 1, w[4], w[2]);
                    }
                    else
                    {
                        dst[1] = mix(6, 1, 1, w[4], w[1], w[5]);
                    }
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = mix(3, 1, w[4], w[6]);
                    }
                    else
                    {
                        dst[dst_stride] = mix(6, 1, 1, w[4], w[7], w[3]);
                    }
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = mix(3, 1, w[4], w[8]);
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 55:
            case 23:
                {
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[0] = mix(3, 1, w[4], w[3]);
                        dst[1] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(5, 2, 1, w[4], w[1], w[3]);
                        dst[1] = mix(2, 3, 3, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[7]);
                    break;
                }
            case 182:
            case 150:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[3]);
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = w[4];
                        dst[dst_stride + 1] = mix(3, 1, w[4], w[7]);
                    }
                    else
                    {
                        dst[1] = mix(2, 3, 3, w[4], w[1], w[5]);
                        dst[dst_stride + 1] = mix(5, 2, 1, w[4], w[5], w[7]);
                    }
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    break;
                }
            case 213:
            case 212:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[1] = mix(5, 2, 1, w[4], w[5], w[1]);
                        dst[dst_stride + 1] = mix(2, 3, 3, w[4], w[5], w[7]);
                    }
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[3]);
                    break;
                }
            case 241:
            case 240:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[1]);
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[7], w[3]);
                        dst[dst_stride + 1] = mix(2, 3, 3, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 236:
            case 232:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = mix(3, 1, w[4], w[5]);
                    }
                    else
                    {
                        dst[dst_stride] = mix(2, 3, 3, w[4], w[7], w[3]);
                        dst[dst_stride + 1] = mix(5, 2, 1, w[4], w[7], w[5]);
                    }
                    break;
                }
            case 109:
            case 105:
                {
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[0] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(5, 2, 1, w[4], w[3], w[1]);
                        dst[dst_stride] = mix(2, 3, 3, w[4], w[7], w[3]);
                    }
                    dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[5]);
                    break;
                }
            case 171:
            case 43:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                        dst[dst_stride] = mix(3, 1, w[4], w[7]);
                    }
                    else
                    {
                        dst[0] = mix(2, 3, 3, w[4], w[3], w[1]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[7]);
                    }
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[5]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    break;
                }
            case 143:
            case 15:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                        dst[1] = mix(3, 1, w[4], w[5]);
                    }
                    else
                    {
                        dst[0] = mix(2, 3, 3, w[4], w[3], w[1]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[7]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    break;
                }
            case 124:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[1]);
                    dst[1] = mix(3, 1, w[4], w[1]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = w[4];
                    }
                    else
                    {
                        dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    }
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[8]);
                    break;
                }
            case 203:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    }
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[5]);
                    dst[dst_stride] = mix(3, 1, w[4], w[6]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[5]);
                    break;
                }
            case 62:
                {
                    dst[0] = mix(3, 1, w[4], w[0]);
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = w[4];
                    }
                    else
                    {
                        dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(3, 1, w[4], w[7]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[7]);
                    break;
                }
            case 211:
                {
                    dst[0] = mix(3, 1, w[4], w[3]);
                    dst[1] = mix(3, 1, w[4], w[2]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[3]);
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 118:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[3]);
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = w[4];
                    }
                    else
                    {
                        dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(3, 1, w[4], w[3]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[8]);
                    break;
                }
            case 217:
                {
                    dst[0] = mix(3, 1, w[4], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[1]);
                    dst[dst_stride] = mix(3, 1, w[4], w[6]);
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 110:
                {
                    dst[0] = mix(3, 1, w[4], w[0]);
                    dst[1] = mix(3, 1, w[4], w[5]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = w[4];
                    }
                    else
                    {
                        dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    }
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[5]);
                    break;
                }
            case 155:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    }
                    dst[1] = mix(3, 1, w[4], w[2]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[7]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[7]);
                    break;
                }
            case 188:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[1]);
                    dst[1] = mix(3, 1, w[4], w[1]);
                    dst[dst_stride] = mix(3, 1, w[4], w[7]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[7]);
                    break;
                }
            case 185:
                {
                    dst[0] = mix(3, 1, w[4], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[1]);
                    dst[dst_stride] = mix(3, 1, w[4], w[7]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[7]);
                    break;
                }
            case 61:
                {
                    dst[0] = mix(3, 1, w[4], w[1]);
                    dst[1] = mix(3, 1, w[4], w[1]);
                    dst[dst_stride] = mix(3, 1, w[4], w[7]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[7]);
                    break;
                }
            case 157:
                {
                    dst[0] = mix(3, 1, w[4], w[1]);
                    dst[1] = mix(3, 1, w[4], w[1]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[7]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[7]);
                    break;
                }
            case 103:
                {
                    dst[0] = mix(3, 1, w[4], w[3]);
                    dst[1] = mix(3, 1, w[4], w[5]);
                    dst[dst_stride] = mix(3, 1, w[4], w[3]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[5]);
                    break;
                }
            case 227:
                {
                    dst[0] = mix(3, 1, w[4], w[3]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[5]);
                    dst[dst_stride] = mix(3, 1, w[4], w[3]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[5]);
                    break;
                }
            case 230:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[3]);
                    dst[1] = mix(3, 1, w[4], w[5]);
                    dst[dst_stride] = mix(3, 1, w[4], w[3]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[5]);
                    break;
                }
            case 199:
                {
                    dst[0] = mix(3, 1, w[4], w[3]);
                    dst[1] = mix(3, 1, w[4], w[5]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[3]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[5]);
                    break;
                }
            case 220:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[1]);
                    dst[1] = mix(3, 1, w[4], w[1]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = mix(3, 1, w[4], w[6]);
                    }
                    else
                    {
                        dst[dst_stride] = mix(6, 1, 1, w[4], w[7], w[3]);
                    }
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 158:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                    }
                    else
                    {
                        dst[0] = mix(6, 1, 1, w[4], w[3], w[1]);
                    }
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = w[4];
                    }
                    else
                    {
                        dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[7]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[7]);
                    break;
                }
            case 234:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                    }
                    else
                    {
                        dst[0] = mix(6, 1, 1, w[4], w[3], w[1]);
                    }
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[5]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = w[4];
                    }
                    else
                    {
                        dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    }
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[5]);
                    break;
                }
            case 242:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[3]);
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = mix(3, 1, w[4], w[2]);
                    }
                    else
                    {
                        dst[1] = mix(6, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(3, 1, w[4], w[3]);
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 59:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    }
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = mix(3, 1, w[4], w[2]);
                    }
                    else
                    {
                        dst[1] = mix(6, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(3, 1, w[4], w[7]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[7]);
                    break;
                }
            case 121:
                {
                    dst[0] = mix(3, 1, w[4], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[1]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = w[4];
                    }
                    else
                    {
                        dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    }
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = mix(3, 1, w[4], w[8]);
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 87:
                {
                    dst[0] = mix(3, 1, w[4], w[3]);
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = w[4];
                    }
                    else
                    {
                        dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[3]);
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = mix(3, 1, w[4], w[8]);
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 79:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    }
                    dst[1] = mix(3, 1, w[4], w[5]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = mix(3, 1, w[4], w[6]);
                    }
                    else
                    {
                        dst[dst_stride] = mix(6, 1, 1, w[4], w[7], w[3]);
                    }
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[5]);
                    break;
                }
            case 122:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                    }
                    else
                    {
                        dst[0] = mix(6, 1, 1, w[4], w[3], w[1]);
                    }
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = mix(3, 1, w[4], w[2]);
                    }
                    else
                    {
                        dst[1] = mix(6, 1, 1, w[4], w[1], w[5]);
                    }
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = w[4];
                    }
                    else
                    {
                        dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    }
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = mix(3, 1, w[4], w[8]);
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 94:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                    }
                    else
                    {
                        dst[0] = mix(6, 1, 1, w[4], w[3], w[1]);
                    }
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = w[4];
                    }
                    else
                    {
                        dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    }
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = mix(3, 1, w[4], w[6]);
                    }
                    else
                    {
                        dst[dst_stride] = mix(6, 1, 1, w[4], w[7], w[3]);
                    }
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = mix(3, 1, w[4], w[8]);
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 218:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                    }
                    else
                    {
                        dst[0] = mix(6, 1, 1, w[4], w[3], w[1]);
                    }
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = mix(3, 1, w[4], w[2]);
                    }
                    else
                    {
                        dst[1] = mix(6, 1, 1, w[4], w[1], w[5]);
                    }
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = mix(3, 1, w[4], w[6]);
                    }
                    else
                    {
                        dst[dst_stride] = mix(6, 1, 1, w[4], w[7], w[3]);
                    }
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 91:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    }
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = mix(3, 1, w[4], w[2]);
                    }
                    else
                    {
                        dst[1] = mix(6, 1, 1, w[4], w[1], w[5]);
                    }
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = mix(3, 1, w[4], w[6]);
                    }
                    else
                    {
                        dst[dst_stride] = mix(6, 1, 1, w[4], w[7], w[3]);
                    }
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = mix(3, 1, w[4], w[8]);
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 229:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    dst[dst_stride] = mix(3, 1, w[4], w[3]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[5]);
                    break;
                }
            case 167:
                {
                    dst[0] = mix(3, 1, w[4], w[3]);
                    dst[1] = mix(3, 1, w[4], w[5]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    break;
                }
            case 173:
                {
                    dst[0] = mix(3, 1, w[4], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    dst[dst_stride] = mix(3, 1, w[4], w[7]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    break;
                }
            case 181:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    dst[1] = mix(3, 1, w[4], w[1]);
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[7]);
                    break;
                }
            case 186:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                    }
                    else
                    {
                        dst[0] = mix(6, 1, 1, w[4], w[3], w[1]);
                    }
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = mix(3, 1, w[4], w[2]);
                    }
                    else
                    {
                        dst[1] = mix(6, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(3, 1, w[4], w[7]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[7]);
                    break;
                }
            case 115:
                {
                    dst[0] = mix(3, 1, w[4], w[3]);
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = mix(3, 1, w[4], w[2]);
                    }
                    else
                    {
                        dst[1] = mix(6, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(3, 1, w[4], w[3]);
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = mix(3, 1, w[4], w[8]);
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 93:
                {
                    dst[0] = mix(3, 1, w[4], w[1]);
                    dst[1] = mix(3, 1, w[4], w[1]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = mix(3, 1, w[4], w[6]);
                    }
                    else
                    {
                        dst[dst_stride] = mix(6, 1, 1, w[4], w[7], w[3]);
                    }
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = mix(3, 1, w[4], w[8]);
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 206:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                    }
                    else
                    {
                        dst[0] = mix(6, 1, 1, w[4], w[3], w[1]);
                    }
                    dst[1] = mix(3, 1, w[4], w[5]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = mix(3, 1, w[4], w[6]);
                    }
                    else
                    {
                        dst[dst_stride] = mix(6, 1, 1, w[4], w[7], w[3]);
                    }
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[5]);
                    break;
                }
            case 205:
            case 201:
                {
                    dst[0] = mix(3, 1, w[4], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = mix(3, 1, w[4], w[6]);
                    }
                    else
                    {
                        dst[dst_stride] = mix(6, 1, 1, w[4], w[7], w[3]);
                    }
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[5]);
                    break;
                }
            case 174:
            case 46:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                    }
                    else
                    {
                        dst[0] = mix(6, 1, 1, w[4], w[3], w[1]);
                    }
                    dst[1] = mix(3, 1, w[4], w[5]);
                    dst[dst_stride] = mix(3, 1, w[4], w[7]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    break;
                }
            case 179:
            case 147:
                {
                    dst[0] = mix(3, 1, w[4], w[3]);
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = mix(3, 1, w[4], w[2]);
                    }
                    else
                    {
                        dst[1] = mix(6, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[7]);
                    break;
                }
            case 117:
            case 116:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    dst[1] = mix(3, 1, w[4], w[1]);
                    dst[dst_stride] = mix(3, 1, w[4], w[3]);
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = mix(3, 1, w[4], w[8]);
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 189:
                {
                    dst[0] = mix(3, 1, w[4], w[1]);
                    dst[1] = mix(3, 1, w[4], w[1]);
                    dst[dst_stride] = mix(3, 1, w[4], w[7]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[7]);
                    break;
                }
            case 231:
                {
                    dst[0] = mix(3, 1, w[4], w[3]);
                    dst[1] = mix(3, 1, w[4], w[5]);
                    dst[dst_stride] = mix(3, 1, w[4], w[3]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[5]);
                    break;
                }
            case 126:
                {
                    dst[0] = mix(3, 1, w[4], w[0]);
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = w[4];
                    }
                    else
                    {
                        dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    }
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = w[4];
                    }
                    else
                    {
                        dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    }
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[8]);
                    break;
                }
            case 219:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    }
                    dst[1] = mix(3, 1, w[4], w[2]);
                    dst[dst_stride] = mix(3, 1, w[4], w[6]);
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 125:
                {
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[0] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(5, 2, 1, w[4], w[3], w[1]);
                        dst[dst_stride] = mix(2, 3, 3, w[4], w[7], w[3]);
                    }
                    dst[1] = mix(3, 1, w[4], w[1]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[8]);
                    break;
                }
            case 221:
                {
                    dst[0] = mix(3, 1, w[4], w[1]);
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[1] = mix(5, 2, 1, w[4], w[5], w[1]);
                        dst[dst_stride + 1] = mix(2, 3, 3, w[4], w[5], w[7]);
                    }
                    dst[dst_stride] = mix(3, 1, w[4], w[6]);
                    break;
                }
            case 207:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                        dst[1] = mix(3, 1, w[4], w[5]);
                    }
                    else
                    {
                        dst[0] = mix(2, 3, 3, w[4], w[3], w[1]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(3, 1, w[4], w[6]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[5]);
                    break;
                }
            case 238:
                {
                    dst[0] = mix(3, 1, w[4], w[0]);
                    dst[1] = mix(3, 1, w[4], w[5]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = mix(3, 1, w[4], w[5]);
                    }
                    else
                    {
                        dst[dst_stride] = mix(2, 3, 3, w[4], w[7], w[3]);
                        dst[dst_stride + 1] = mix(5, 2, 1, w[4], w[7], w[5]);
                    }
                    break;
                }
            case 190:
                {
                    dst[0] = mix(3, 1, w[4], w[0]);
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = w[4];
                        dst[dst_stride + 1] = mix(3, 1, w[4], w[7]);
                    }
                    else
                    {
                        dst[1] = mix(2, 3, 3, w[4], w[1], w[5]);
                        dst[dst_stride + 1] = mix(5, 2, 1, w[4], w[5], w[7]);
                    }
                    dst[dst_stride] = mix(3, 1, w[4], w[7]);
                    break;
                }
            case 187:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                        dst[dst_stride] = mix(3, 1, w[4], w[7]);
                    }
                    else
                    {
                        dst[0] = mix(2, 3, 3, w[4], w[3], w[1]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[7]);
                    }
                    dst[1] = mix(3, 1, w[4], w[2]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[7]);
                    break;
                }
            case 243:
                {
                    dst[0] = mix(3, 1, w[4], w[3]);
                    dst[1] = mix(3, 1, w[4], w[2]);
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[7], w[3]);
                        dst[dst_stride + 1] = mix(2, 3, 3, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 119:
                {
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[0] = mix(3, 1, w[4], w[3]);
                        dst[1] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(5, 2, 1, w[4], w[1], w[3]);
                        dst[1] = mix(2, 3, 3, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(3, 1, w[4], w[3]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[8]);
                    break;
                }
            case 237:
            case 233:
                {
                    dst[0] = mix(3, 1, w[4], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = w[4];
                    }
                    else
                    {
                        dst[dst_stride] = mix(14, 1, 1, w[4], w[7], w[3]);
                    }
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[5]);
                    break;
                }
            case 175:
            case 47:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(14, 1, 1, w[4], w[3], w[1]);
                    }
                    dst[1] = mix(3, 1, w[4], w[5]);
                    dst[dst_stride] = mix(3, 1, w[4], w[7]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    break;
                }
            case 183:
            case 151:
                {
                    dst[0] = mix(3, 1, w[4], w[3]);
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = w[4];
                    }
                    else
                    {
                        dst[1] = mix(14, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[7]);
                    break;
                }
            case 245:
            case 244:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    dst[1] = mix(3, 1, w[4], w[1]);
                    dst[dst_stride] = mix(3, 1, w[4], w[3]);
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(14, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 250:
                {
                    dst[0] = mix(3, 1, w[4], w[0]);
                    dst[1] = mix(3, 1, w[4], w[2]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = w[4];
                    }
                    else
                    {
                        dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    }
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 123:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    }
                    dst[1] = mix(3, 1, w[4], w[2]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = w[4];
                    }
                    else
                    {
                        dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    }
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[8]);
                    break;
                }
            case 95:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    }
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = w[4];
                    }
                    else
                    {
                        dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(3, 1, w[4], w[6]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[8]);
                    break;
                }
            case 222:
                {
                    dst[0] = mix(3, 1, w[4], w[0]);
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = w[4];
                    }
                    else
                    {
                        dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(3, 1, w[4], w[6]);
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 252:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[1]);
                    dst[1] = mix(3, 1, w[4], w[1]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = w[4];
                    }
                    else
                    {
                        dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    }
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(14, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 249:
                {
                    dst[0] = mix(3, 1, w[4], w[1]);
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[1]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = w[4];
                    }
                    else
                    {
                        dst[dst_stride] = mix(14, 1, 1, w[4], w[7], w[3]);
                    }
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 235:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    }
                    dst[1] = mix(2, 1, 1, w[4], w[2], w[5]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = w[4];
                    }
                    else
                    {
                        dst[dst_stride] = mix(14, 1, 1, w[4], w[7], w[3]);
                    }
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[5]);
                    break;
                }
            case 111:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(14, 1, 1, w[4], w[3], w[1]);
                    }
                    dst[1] = mix(3, 1, w[4], w[5]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = w[4];
                    }
                    else
                    {
                        dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    }
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[5]);
                    break;
                }
            case 63:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(14, 1, 1, w[4], w[3], w[1]);
                    }
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = w[4];
                    }
                    else
                    {
                        dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(3, 1, w[4], w[7]);
                    dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[8], w[7]);
                    break;
                }
            case 159:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    }
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = w[4];
                    }
                    else
                    {
                        dst[1] = mix(14, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[7]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[7]);
                    break;
                }
            case 215:
                {
                    dst[0] = mix(3, 1, w[4], w[3]);
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = w[4];
                    }
                    else
                    {
                        dst[1] = mix(14, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(2, 1, 1, w[4], w[6], w[3]);
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 246:
                {
                    dst[0] = mix(2, 1, 1, w[4], w[0], w[3]);
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = w[4];
                    }
                    else
                    {
                        dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(3, 1, w[4], w[3]);
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(14, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 254:
                {
                    dst[0] = mix(3, 1, w[4], w[0]);
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = w[4];
                    }
                    else
                    {
                        dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    }
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = w[4];
                    }
                    else
                    {
                        dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    }
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(14, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 253:
                {
                    dst[0] = mix(3, 1, w[4], w[1]);
                    dst[1] = mix(3, 1, w[4], w[1]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = w[4];
                    }
                    else
                    {
                        dst[dst_stride] = mix(14, 1, 1, w[4], w[7], w[3]);
                    }
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(14, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 251:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    }
                    dst[1] = mix(3, 1, w[4], w[2]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = w[4];
                    }
                    else
                    {
                        dst[dst_stride] = mix(14, 1, 1, w[4], w[7], w[3]);
                    }
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 239:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(14, 1, 1, w[4], w[3], w[1]);
                    }
                    dst[1] = mix(3, 1, w[4], w[5]);
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = w[4];
                    }
                    else
                    {
                        dst[dst_stride] = mix(14, 1, 1, w[4], w[7], w[3]);
                    }
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[5]);
                    break;
                }
            case 127:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(14, 1, 1, w[4], w[3], w[1]);
                    }
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = w[4];
                    }
                    else
                    {
                        dst[1] = mix(2, 1, 1, w[4], w[1], w[5]);
                    }
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = w[4];
                    }
                    else
                    {
                        dst[dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                    }
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[8]);
                    break;
                }
            case 191:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(14, 1, 1, w[4], w[3], w[1]);
                    }
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = w[4];
                    }
                    else
                    {
                        dst[1] = mix(14, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(3, 1, w[4], w[7]);
                    dst[dst_stride + 1] = mix(3, 1, w[4], w[7]);
                    break;
                }
            case 223:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                    }
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = w[4];
                    }
                    else
                    {
                        dst[1] = mix(14, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(3, 1, w[4], w[6]);
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(2, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 247:
                {
                    dst[0] = mix(3, 1, w[4], w[3]);
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = w[4];
                    }
                    else
                    {
                        dst[1] = mix(14, 1, 1, w[4], w[1], w[5]);
                    }
                    dst[dst_stride] = mix(3, 1, w[4], w[3]);
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(14, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            case 255:
                {
                    if (diff(w[3], w[1], trY, trU, trV, trA))
                    {
                        dst[0] = w[4];
                    }
                    else
                    {
                        dst[0] = mix(14, 1, 1, w[4], w[3], w[1]);
                    }
                    if (diff(w[1], w[5], trY, trU, trV, trA))
                    {
                        dst[1] = w[4];
                    }
                    else
                    {
                        dst[1] = mix(14, 1, 1, w[4], w[1], w[5]);
                    }
                    if (diff(w[7], w[3], trY, trU, trV, trA))
                    {
                        dst[dst_stride] = w[4];
                    }
                    else
                    {
                        dst[dst_stride] = mix(14, 1, 1, w[4], w[7], w[3]);
                    }
                    if (diff(w[5], w[7], trY, trU, trV, trA))
                    {
                        dst[dst_stride + 1] = w[4];
                    }
                    else
                    {
                        dst[dst_stride + 1] = mix(14, 1, 1, w[4], w[5], w[7]);
                    }
                    break;
                }
            }
            src++;
            dst+=2;
        }
        src += src_stride - src_size.x;
        dst += dst_stride - src_size.x * 2;
        dst += dst_stride;
    }
}

static void hq3xProcess(const uint32_t* src, uint32_t* dst, sp::Vector2i src_size, int src_stride, HQ2xConfig config)
{
    uint32_t trY = 48;
    uint32_t trU = 7;
    uint32_t trV = 6;
    uint32_t trA = 0;
    
    trY <<= 16;
    trU <<= 8;
    int dst_stride = src_stride * 3;

    int prevline_offset, nextline_offset;
    uint32_t w[9];

    for(int y = 0; y<src_size.y; y++)
    {
        if (y > 0)
            prevline_offset = -src_stride;
        else if (config.out_of_bounds == HQ2xConfig::OutOfBounds::Wrap)
            prevline_offset = src_stride * (src_size.y - 1);
        else
            prevline_offset = 0;
        if (y < src_size.y - 1)
            nextline_offset = src_stride;
        else if (config.out_of_bounds == HQ2xConfig::OutOfBounds::Wrap)
            nextline_offset = -(src_stride * (src_size.y - 1));
        else
            nextline_offset = 0;

        for(int x = 0; x<src_size.x; x++)
        {
            w[1] = src[prevline_offset];
            w[4] = src[0];
            w[7] = src[nextline_offset];

            if (x > 0)
            {
                w[0] = src[prevline_offset - 1];
                w[3] = src[-1];
                w[6] = src[nextline_offset - 1];
            }
            else if (config.out_of_bounds == HQ2xConfig::OutOfBounds::Wrap)
            {
                w[0] = src[prevline_offset + src_size.x - 1];
                w[3] = src[src_size.x - 1];
                w[6] = src[nextline_offset + src_size.x - 1];
            }
            else if (config.out_of_bounds == HQ2xConfig::OutOfBounds::Clamp)
            {
                w[0] = w[1];
                w[3] = w[4];
                w[6] = w[7];
            }
            else
            {
                w[0] = 0;
                w[3] = 0;
                w[6] = 0;
            }

            if (x < src_size.x - 1)
            {
                w[2] = src[prevline_offset + 1];
                w[5] = src[1];
                w[8] = src[nextline_offset + 1];
            }
            else if (config.out_of_bounds == HQ2xConfig::OutOfBounds::Wrap)
            {
                w[2] = src[prevline_offset - src_size.x + 1];
                w[5] = src[-src_size.x + 1];
                w[8] = src[nextline_offset - src_size.x + 1];
            }
            else if (config.out_of_bounds == HQ2xConfig::OutOfBounds::Clamp)
            {
                w[2] = w[1];
                w[5] = w[4];
                w[8] = w[7];
            }
            else if (config.out_of_bounds == HQ2xConfig::OutOfBounds::Transparent)
            {
                w[2] = 0;
                w[5] = 0;
                w[8] = 0;
            }

            if (y == 0 && config.out_of_bounds == HQ2xConfig::OutOfBounds::Transparent)
                w[0] = w[1] = w[2] = 0;
            if (y == src_size.y - 1 && config.out_of_bounds == HQ2xConfig::OutOfBounds::Transparent)
                w[6] = w[7] = w[8] = 0;

            int pattern = 0;
            int flag = 1;

            for(int k = 0; k < 9; k++)
            {
                if (k == 4) continue;

                if (w[k] != w[4])
                {
                    if (diff(w[4], w[k], trY, trU, trV, trA))
                        pattern |= flag;
                }
                flag <<= 1;
            }

            switch (pattern)
            {
                case 0:
                case 1:
                case 4:
                case 32:
                case 128:
                case 5:
                case 132:
                case 160:
                case 33:
                case 129:
                case 36:
                case 133:
                case 164:
                case 161:
                case 37:
                case 165:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        break;
                    }
                case 2:
                case 34:
                case 130:
                case 162:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        break;
                    }
                case 16:
                case 17:
                case 48:
                case 49:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 64:
                case 65:
                case 68:
                case 69:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 8:
                case 12:
                case 136:
                case 140:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        break;
                    }
                case 3:
                case 35:
                case 131:
                case 163:
                    {
                        dst[0] = mix(3, 1, w[4], w[3]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        break;
                    }
                case 6:
                case 38:
                case 134:
                case 166:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        break;
                    }
                case 20:
                case 21:
                case 52:
                case 53:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 144:
                case 145:
                case 176:
                case 177:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[7]);
                        break;
                    }
                case 192:
                case 193:
                case 196:
                case 197:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        break;
                    }
                case 96:
                case 97:
                case 100:
                case 101:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 40:
                case 44:
                case 168:
                case 172:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        break;
                    }
                case 9:
                case 13:
                case 137:
                case 141:
                    {
                        dst[0] = mix(3, 1, w[4], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        break;
                    }
                case 18:
                case 50:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[1] = w[4];
                            dst[2] = mix(3, 1, w[4], w[2]);
                            dst[dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[1] = mix(7, 1, w[4], w[1]);
                            dst[2] = mix(2, 7, 7, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        }
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 80:
                case 81:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                            dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                            dst[dst_stride + dst_stride + 2] = mix(2, 7, 7, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 72:
                case 76:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride] = w[4];
                            dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                            dst[dst_stride + dst_stride + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                            dst[dst_stride + dst_stride] = mix(2, 7, 7, w[4], w[7], w[3]);
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                        }
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 10:
                case 138:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(3, 1, w[4], w[0]);
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 7, 7, w[4], w[3], w[1]);
                            dst[1] = mix(7, 1, w[4], w[1]);
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        break;
                    }
                case 66:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 24:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 7:
                case 39:
                case 135:
                    {
                        dst[0] = mix(3, 1, w[4], w[3]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        break;
                    }
                case 148:
                case 149:
                case 180:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[7]);
                        break;
                    }
                case 224:
                case 228:
                case 225:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        break;
                    }
                case 41:
                case 169:
                case 45:
                    {
                        dst[0] = mix(3, 1, w[4], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        break;
                    }
                case 22:
                case 54:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[1] = w[4];
                            dst[2] = w[4];
                            dst[dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[1] = mix(7, 1, w[4], w[1]);
                            dst[2] = mix(2, 7, 7, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        }
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 208:
                case 209:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                            dst[dst_stride + dst_stride + 2] = mix(2, 7, 7, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 104:
                case 108:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride] = w[4];
                            dst[dst_stride + dst_stride] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                            dst[dst_stride + dst_stride] = mix(2, 7, 7, w[4], w[7], w[3]);
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                        }
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 11:
                case 139:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 7, 7, w[4], w[3], w[1]);
                            dst[1] = mix(7, 1, w[4], w[1]);
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        break;
                    }
                case 19:
                case 51:
                    {
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[0] = mix(3, 1, w[4], w[3]);
                            dst[1] = w[4];
                            dst[2] = mix(3, 1, w[4], w[2]);
                            dst[dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                            dst[1] = mix(3, 1, w[1], w[4]);
                            dst[2] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        }
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 146:
                case 178:
                    {
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[1] = w[4];
                            dst[2] = mix(3, 1, w[4], w[2]);
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[7]);
                        }
                        else
                        {
                            dst[1] = mix(3, 1, w[4], w[1]);
                            dst[2] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 2] = mix(3, 1, w[5], w[4]);
                            dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        }
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        break;
                    }
                case 84:
                case 85:
                    {
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[1]);
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                            dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        }
                        else
                        {
                            dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = mix(3, 1, w[5], w[4]);
                            dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                            dst[dst_stride + dst_stride + 2] = mix(1, 1, w[5], w[7]);
                        }
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        break;
                    }
                case 112:
                case 113:
                    {
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride + dst_stride + 1] = w[4];
                            dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                            dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                            dst[dst_stride + dst_stride + 1] = mix(3, 1, w[7], w[4]);
                            dst[dst_stride + dst_stride + 2] = mix(1, 1, w[5], w[7]);
                        }
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        break;
                    }
                case 200:
                case 204:
                    {
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride] = w[4];
                            dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                            dst[dst_stride + dst_stride + 1] = w[4];
                            dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        }
                        else
                        {
                            dst[dst_stride] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride + dst_stride] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride + dst_stride + 1] = mix(3, 1, w[7], w[4]);
                            dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        }
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        break;
                    }
                case 73:
                case 77:
                    {
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[0] = mix(3, 1, w[4], w[1]);
                            dst[dst_stride] = w[4];
                            dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                            dst[dst_stride + dst_stride + 1] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                            dst[dst_stride] = mix(3, 1, w[3], w[4]);
                            dst[dst_stride + dst_stride] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        }
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 42:
                case 170:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(3, 1, w[4], w[0]);
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                            dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[7]);
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[3], w[1]);
                            dst[1] = mix(3, 1, w[4], w[1]);
                            dst[dst_stride] = mix(3, 1, w[3], w[4]);
                            dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        break;
                    }
                case 14:
                case 142:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(3, 1, w[4], w[0]);
                            dst[1] = w[4];
                            dst[2] = mix(3, 1, w[4], w[5]);
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[3], w[1]);
                            dst[1] = mix(3, 1, w[1], w[4]);
                            dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                            dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        }
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        break;
                    }
                case 67:
                    {
                        dst[0] = mix(3, 1, w[4], w[3]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 70:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 28:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 152:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[7]);
                        break;
                    }
                case 194:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        break;
                    }
                case 98:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 56:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 25:
                    {
                        dst[0] = mix(3, 1, w[4], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 26:
                case 31:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 7, 7, w[4], w[3], w[1]);
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                        }
                        dst[1] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                            dst[dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(2, 7, 7, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        }
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 82:
                case 214:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[1] = w[4];
                            dst[2] = w[4];
                        }
                        else
                        {
                            dst[1] = mix(7, 1, w[4], w[1]);
                            dst[2] = mix(2, 7, 7, w[4], w[1], w[5]);
                        }
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride + 1] = w[4];
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                            dst[dst_stride + dst_stride + 2] = mix(2, 7, 7, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 88:
                case 248:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride + 1] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride] = w[4];
                            dst[dst_stride + dst_stride] = w[4];
                        }
                        else
                        {
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                            dst[dst_stride + dst_stride] = mix(2, 7, 7, w[4], w[7], w[3]);
                        }
                        dst[dst_stride + dst_stride + 1] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                            dst[dst_stride + dst_stride + 2] = mix(2, 7, 7, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 74:
                case 107:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 7, 7, w[4], w[3], w[1]);
                            dst[1] = mix(7, 1, w[4], w[1]);
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + dst_stride] = mix(2, 7, 7, w[4], w[7], w[3]);
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                        }
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 27:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 7, 7, w[4], w[3], w[1]);
                            dst[1] = mix(7, 1, w[4], w[1]);
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 86:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[1] = w[4];
                            dst[2] = w[4];
                            dst[dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[1] = mix(7, 1, w[4], w[1]);
                            dst[2] = mix(2, 7, 7, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        }
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 216:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                            dst[dst_stride + dst_stride + 2] = mix(2, 7, 7, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 106:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride] = w[4];
                            dst[dst_stride + dst_stride] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                            dst[dst_stride + dst_stride] = mix(2, 7, 7, w[4], w[7], w[3]);
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                        }
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 30:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[1] = w[4];
                            dst[2] = w[4];
                            dst[dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[1] = mix(7, 1, w[4], w[1]);
                            dst[2] = mix(2, 7, 7, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        }
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 210:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                            dst[dst_stride + dst_stride + 2] = mix(2, 7, 7, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 120:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride] = w[4];
                            dst[dst_stride + dst_stride] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                            dst[dst_stride + dst_stride] = mix(2, 7, 7, w[4], w[7], w[3]);
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                        }
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 75:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 7, 7, w[4], w[3], w[1]);
                            dst[1] = mix(7, 1, w[4], w[1]);
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 29:
                    {
                        dst[0] = mix(3, 1, w[4], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 198:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        break;
                    }
                case 184:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[7]);
                        break;
                    }
                case 99:
                    {
                        dst[0] = mix(3, 1, w[4], w[3]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 57:
                    {
                        dst[0] = mix(3, 1, w[4], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 71:
                    {
                        dst[0] = mix(3, 1, w[4], w[3]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 156:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[7]);
                        break;
                    }
                case 226:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        break;
                    }
                case 60:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 195:
                    {
                        dst[0] = mix(3, 1, w[4], w[3]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        break;
                    }
                case 102:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 153:
                    {
                        dst[0] = mix(3, 1, w[4], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[7]);
                        break;
                    }
                case 58:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(3, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        }
                        dst[1] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        }
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 83:
                    {
                        dst[0] = mix(3, 1, w[4], w[3]);
                        dst[1] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        }
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 92:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        dst[dst_stride + dst_stride + 1] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 202:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(3, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        }
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        break;
                    }
                case 78:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(3, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        }
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 154:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(3, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        }
                        dst[1] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        }
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[7]);
                        break;
                    }
                case 114:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        }
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 89:
                    {
                        dst[0] = mix(3, 1, w[4], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        dst[dst_stride + dst_stride + 1] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 90:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(3, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        }
                        dst[1] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        }
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        dst[dst_stride + dst_stride + 1] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 55:
                case 23:
                    {
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[0] = mix(3, 1, w[4], w[3]);
                            dst[1] = w[4];
                            dst[2] = w[4];
                            dst[dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                            dst[1] = mix(3, 1, w[1], w[4]);
                            dst[2] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        }
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 182:
                case 150:
                    {
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[1] = w[4];
                            dst[2] = w[4];
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[7]);
                        }
                        else
                        {
                            dst[1] = mix(3, 1, w[4], w[1]);
                            dst[2] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 2] = mix(3, 1, w[5], w[4]);
                            dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        }
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        break;
                    }
                case 213:
                case 212:
                    {
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[1]);
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = mix(3, 1, w[5], w[4]);
                            dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                            dst[dst_stride + dst_stride + 2] = mix(1, 1, w[5], w[7]);
                        }
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        break;
                    }
                case 241:
                case 240:
                    {
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride + dst_stride + 1] = w[4];
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                            dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                            dst[dst_stride + dst_stride + 1] = mix(3, 1, w[7], w[4]);
                            dst[dst_stride + dst_stride + 2] = mix(1, 1, w[5], w[7]);
                        }
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        break;
                    }
                case 236:
                case 232:
                    {
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride] = w[4];
                            dst[dst_stride + dst_stride] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                            dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        }
                        else
                        {
                            dst[dst_stride] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride + dst_stride] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride + dst_stride + 1] = mix(3, 1, w[7], w[4]);
                            dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        }
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        break;
                    }
                case 109:
                case 105:
                    {
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[0] = mix(3, 1, w[4], w[1]);
                            dst[dst_stride] = w[4];
                            dst[dst_stride + dst_stride] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                            dst[dst_stride] = mix(3, 1, w[3], w[4]);
                            dst[dst_stride + dst_stride] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        }
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 171:
                case 43:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                            dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[7]);
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[3], w[1]);
                            dst[1] = mix(3, 1, w[4], w[1]);
                            dst[dst_stride] = mix(3, 1, w[3], w[4]);
                            dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        break;
                    }
                case 143:
                case 15:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[2] = mix(3, 1, w[4], w[5]);
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[3], w[1]);
                            dst[1] = mix(3, 1, w[1], w[4]);
                            dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                            dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        }
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        break;
                    }
                case 124:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride] = w[4];
                            dst[dst_stride + dst_stride] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                            dst[dst_stride + dst_stride] = mix(2, 7, 7, w[4], w[7], w[3]);
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                        }
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 203:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 7, 7, w[4], w[3], w[1]);
                            dst[1] = mix(7, 1, w[4], w[1]);
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        break;
                    }
                case 62:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[1] = w[4];
                            dst[2] = w[4];
                            dst[dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[1] = mix(7, 1, w[4], w[1]);
                            dst[2] = mix(2, 7, 7, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        }
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 211:
                    {
                        dst[0] = mix(3, 1, w[4], w[3]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                            dst[dst_stride + dst_stride + 2] = mix(2, 7, 7, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 118:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[1] = w[4];
                            dst[2] = w[4];
                            dst[dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[1] = mix(7, 1, w[4], w[1]);
                            dst[2] = mix(2, 7, 7, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        }
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 217:
                    {
                        dst[0] = mix(3, 1, w[4], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                            dst[dst_stride + dst_stride + 2] = mix(2, 7, 7, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 110:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride] = w[4];
                            dst[dst_stride + dst_stride] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                            dst[dst_stride + dst_stride] = mix(2, 7, 7, w[4], w[7], w[3]);
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                        }
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 155:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 7, 7, w[4], w[3], w[1]);
                            dst[1] = mix(7, 1, w[4], w[1]);
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[7]);
                        break;
                    }
                case 188:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[7]);
                        break;
                    }
                case 185:
                    {
                        dst[0] = mix(3, 1, w[4], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[7]);
                        break;
                    }
                case 61:
                    {
                        dst[0] = mix(3, 1, w[4], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 157:
                    {
                        dst[0] = mix(3, 1, w[4], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[7]);
                        break;
                    }
                case 103:
                    {
                        dst[0] = mix(3, 1, w[4], w[3]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 227:
                    {
                        dst[0] = mix(3, 1, w[4], w[3]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        break;
                    }
                case 230:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        break;
                    }
                case 199:
                    {
                        dst[0] = mix(3, 1, w[4], w[3]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        break;
                    }
                case 220:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                            dst[dst_stride + dst_stride + 2] = mix(2, 7, 7, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 158:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(3, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        }
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[1] = w[4];
                            dst[2] = w[4];
                            dst[dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[1] = mix(7, 1, w[4], w[1]);
                            dst[2] = mix(2, 7, 7, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        }
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[7]);
                        break;
                    }
                case 234:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(3, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        }
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride] = w[4];
                            dst[dst_stride + dst_stride] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                            dst[dst_stride + dst_stride] = mix(2, 7, 7, w[4], w[7], w[3]);
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                        }
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        break;
                    }
                case 242:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        }
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[3]);
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                            dst[dst_stride + dst_stride + 2] = mix(2, 7, 7, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 59:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 7, 7, w[4], w[3], w[1]);
                            dst[1] = mix(7, 1, w[4], w[1]);
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                        }
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        }
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 121:
                    {
                        dst[0] = mix(3, 1, w[4], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride] = w[4];
                            dst[dst_stride + dst_stride] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                            dst[dst_stride + dst_stride] = mix(2, 7, 7, w[4], w[7], w[3]);
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                        }
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 87:
                    {
                        dst[0] = mix(3, 1, w[4], w[3]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[1] = w[4];
                            dst[2] = w[4];
                            dst[dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[1] = mix(7, 1, w[4], w[1]);
                            dst[2] = mix(2, 7, 7, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        }
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 79:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 7, 7, w[4], w[3], w[1]);
                            dst[1] = mix(7, 1, w[4], w[1]);
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                        }
                        dst[2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 122:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(3, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        }
                        dst[1] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        }
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride] = w[4];
                            dst[dst_stride + dst_stride] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                            dst[dst_stride + dst_stride] = mix(2, 7, 7, w[4], w[7], w[3]);
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                        }
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 94:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(3, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        }
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[1] = w[4];
                            dst[2] = w[4];
                            dst[dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[1] = mix(7, 1, w[4], w[1]);
                            dst[2] = mix(2, 7, 7, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        }
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        dst[dst_stride + dst_stride + 1] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 218:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(3, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        }
                        dst[1] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        }
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                            dst[dst_stride + dst_stride + 2] = mix(2, 7, 7, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 91:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 7, 7, w[4], w[3], w[1]);
                            dst[1] = mix(7, 1, w[4], w[1]);
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                        }
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        }
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        dst[dst_stride + dst_stride + 1] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 229:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        break;
                    }
                case 167:
                    {
                        dst[0] = mix(3, 1, w[4], w[3]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        break;
                    }
                case 173:
                    {
                        dst[0] = mix(3, 1, w[4], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        break;
                    }
                case 181:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[7]);
                        break;
                    }
                case 186:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(3, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        }
                        dst[1] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        }
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[7]);
                        break;
                    }
                case 115:
                    {
                        dst[0] = mix(3, 1, w[4], w[3]);
                        dst[1] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        }
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 93:
                    {
                        dst[0] = mix(3, 1, w[4], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        dst[dst_stride + dst_stride + 1] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 206:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(3, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        }
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        break;
                    }
                case 205:
                case 201:
                    {
                        dst[0] = mix(3, 1, w[4], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        break;
                    }
                case 174:
                case 46:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(3, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        }
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        break;
                    }
                case 179:
                case 147:
                    {
                        dst[0] = mix(3, 1, w[4], w[3]);
                        dst[1] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        }
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[7]);
                        break;
                    }
                case 117:
                case 116:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 189:
                    {
                        dst[0] = mix(3, 1, w[4], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[7]);
                        break;
                    }
                case 231:
                    {
                        dst[0] = mix(3, 1, w[4], w[3]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        break;
                    }
                case 126:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[1] = w[4];
                            dst[2] = w[4];
                            dst[dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[1] = mix(7, 1, w[4], w[1]);
                            dst[2] = mix(2, 7, 7, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        }
                        dst[dst_stride + 1] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride] = w[4];
                            dst[dst_stride + dst_stride] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                            dst[dst_stride + dst_stride] = mix(2, 7, 7, w[4], w[7], w[3]);
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                        }
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 219:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 7, 7, w[4], w[3], w[1]);
                            dst[1] = mix(7, 1, w[4], w[1]);
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                            dst[dst_stride + dst_stride + 2] = mix(2, 7, 7, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 125:
                    {
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[0] = mix(3, 1, w[4], w[1]);
                            dst[dst_stride] = w[4];
                            dst[dst_stride + dst_stride] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                            dst[dst_stride] = mix(3, 1, w[3], w[4]);
                            dst[dst_stride + dst_stride] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        }
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 221:
                    {
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[1]);
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = mix(3, 1, w[5], w[4]);
                            dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                            dst[dst_stride + dst_stride + 2] = mix(1, 1, w[5], w[7]);
                        }
                        dst[0] = mix(3, 1, w[4], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        break;
                    }
                case 207:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[2] = mix(3, 1, w[4], w[5]);
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[3], w[1]);
                            dst[1] = mix(3, 1, w[1], w[4]);
                            dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                            dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        }
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        break;
                    }
                case 238:
                    {
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride] = w[4];
                            dst[dst_stride + dst_stride] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                            dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        }
                        else
                        {
                            dst[dst_stride] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride + dst_stride] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride + dst_stride + 1] = mix(3, 1, w[7], w[4]);
                            dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        }
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        break;
                    }
                case 190:
                    {
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[1] = w[4];
                            dst[2] = w[4];
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[7]);
                        }
                        else
                        {
                            dst[1] = mix(3, 1, w[4], w[1]);
                            dst[2] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 2] = mix(3, 1, w[5], w[4]);
                            dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        }
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        break;
                    }
                case 187:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                            dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[7]);
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[3], w[1]);
                            dst[1] = mix(3, 1, w[4], w[1]);
                            dst[dst_stride] = mix(3, 1, w[3], w[4]);
                            dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[7]);
                        break;
                    }
                case 243:
                    {
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride + dst_stride + 1] = w[4];
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                            dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                            dst[dst_stride + dst_stride + 1] = mix(3, 1, w[7], w[4]);
                            dst[dst_stride + dst_stride + 2] = mix(1, 1, w[5], w[7]);
                        }
                        dst[0] = mix(3, 1, w[4], w[3]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        break;
                    }
                case 119:
                    {
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[0] = mix(3, 1, w[4], w[3]);
                            dst[1] = w[4];
                            dst[2] = w[4];
                            dst[dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                            dst[1] = mix(3, 1, w[1], w[4]);
                            dst[2] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        }
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 237:
                case 233:
                    {
                        dst[0] = mix(3, 1, w[4], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        break;
                    }
                case 175:
                case 47:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        }
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        break;
                    }
                case 183:
                case 151:
                    {
                        dst[0] = mix(3, 1, w[4], w[3]);
                        dst[1] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        }
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[7]);
                        break;
                    }
                case 245:
                case 244:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 250:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride + 1] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride] = w[4];
                            dst[dst_stride + dst_stride] = w[4];
                        }
                        else
                        {
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                            dst[dst_stride + dst_stride] = mix(2, 7, 7, w[4], w[7], w[3]);
                        }
                        dst[dst_stride + dst_stride + 1] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                            dst[dst_stride + dst_stride + 2] = mix(2, 7, 7, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 123:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 7, 7, w[4], w[3], w[1]);
                            dst[1] = mix(7, 1, w[4], w[1]);
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + dst_stride] = mix(2, 7, 7, w[4], w[7], w[3]);
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                        }
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 95:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 7, 7, w[4], w[3], w[1]);
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                        }
                        dst[1] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                            dst[dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(2, 7, 7, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        }
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 222:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[1] = w[4];
                            dst[2] = w[4];
                        }
                        else
                        {
                            dst[1] = mix(7, 1, w[4], w[1]);
                            dst[2] = mix(2, 7, 7, w[4], w[1], w[5]);
                        }
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride + 1] = w[4];
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                            dst[dst_stride + dst_stride + 2] = mix(2, 7, 7, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 252:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride] = w[4];
                            dst[dst_stride + dst_stride] = w[4];
                        }
                        else
                        {
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                            dst[dst_stride + dst_stride] = mix(2, 7, 7, w[4], w[7], w[3]);
                        }
                        dst[dst_stride + dst_stride + 1] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 249:
                    {
                        dst[0] = mix(3, 1, w[4], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        dst[dst_stride + dst_stride + 1] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                            dst[dst_stride + dst_stride + 2] = mix(2, 7, 7, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 235:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 7, 7, w[4], w[3], w[1]);
                            dst[1] = mix(7, 1, w[4], w[1]);
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        break;
                    }
                case 111:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        }
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + dst_stride] = mix(2, 7, 7, w[4], w[7], w[3]);
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                        }
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 63:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        }
                        dst[1] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                            dst[dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(2, 7, 7, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        }
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 159:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 7, 7, w[4], w[3], w[1]);
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                        }
                        dst[1] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        }
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[7]);
                        break;
                    }
                case 215:
                    {
                        dst[0] = mix(3, 1, w[4], w[3]);
                        dst[1] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        }
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride + 1] = w[4];
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                            dst[dst_stride + dst_stride + 2] = mix(2, 7, 7, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 246:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[1] = w[4];
                            dst[2] = w[4];
                        }
                        else
                        {
                            dst[1] = mix(7, 1, w[4], w[1]);
                            dst[2] = mix(2, 7, 7, w[4], w[1], w[5]);
                        }
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 254:
                    {
                        dst[0] = mix(3, 1, w[4], w[0]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[1] = w[4];
                            dst[2] = w[4];
                        }
                        else
                        {
                            dst[1] = mix(7, 1, w[4], w[1]);
                            dst[2] = mix(2, 7, 7, w[4], w[1], w[5]);
                        }
                        dst[dst_stride + 1] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride] = w[4];
                            dst[dst_stride + dst_stride] = w[4];
                        }
                        else
                        {
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                            dst[dst_stride + dst_stride] = mix(2, 7, 7, w[4], w[7], w[3]);
                        }
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                            dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 253:
                    {
                        dst[0] = mix(3, 1, w[4], w[1]);
                        dst[1] = mix(3, 1, w[4], w[1]);
                        dst[2] = mix(3, 1, w[4], w[1]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        dst[dst_stride + dst_stride + 1] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 251:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 7, 7, w[4], w[3], w[1]);
                            dst[1] = mix(7, 1, w[4], w[1]);
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride + 1] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride] = w[4];
                            dst[dst_stride + dst_stride] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                            dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                        }
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                            dst[dst_stride + dst_stride + 2] = mix(2, 7, 7, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 239:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        }
                        dst[1] = w[4];
                        dst[2] = mix(3, 1, w[4], w[5]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        dst[dst_stride + dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[5]);
                        break;
                    }
                case 127:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                            dst[1] = mix(7, 1, w[4], w[1]);
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                        }
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                            dst[dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(2, 7, 7, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        }
                        dst[dst_stride + 1] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride] = w[4];
                            dst[dst_stride + dst_stride + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + dst_stride] = mix(2, 7, 7, w[4], w[7], w[3]);
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                        }
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[8]);
                        break;
                    }
                case 191:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        }
                        dst[1] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        }
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 1] = mix(3, 1, w[4], w[7]);
                        dst[dst_stride + dst_stride + 2] = mix(3, 1, w[4], w[7]);
                        break;
                    }
                case 223:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 7, 7, w[4], w[3], w[1]);
                            dst[dst_stride] = mix(7, 1, w[4], w[3]);
                        }
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[1] = w[4];
                            dst[2] = w[4];
                            dst[dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[1] = mix(7, 1, w[4], w[1]);
                            dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        }
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[6]);
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride + 1] = w[4];
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + dst_stride + 1] = mix(7, 1, w[4], w[7]);
                            dst[dst_stride + dst_stride + 2] = mix(2, 7, 7, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 247:
                    {
                        dst[0] = mix(3, 1, w[4], w[3]);
                        dst[1] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        }
                        dst[dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + dst_stride] = mix(3, 1, w[4], w[3]);
                        dst[dst_stride + dst_stride + 1] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        }
                        break;
                    }
                case 255:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[3], w[1]);
                        }
                        dst[1] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(2, 1, 1, w[4], w[1], w[5]);
                        }
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + dst_stride] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        dst[dst_stride + dst_stride + 1] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride + dst_stride + 2] = w[4];
                        }
                        else
                        {
                            dst[dst_stride + dst_stride + 2] = mix(2, 1, 1, w[4], w[5], w[7]);
                        }
                        break;
                    }
            }
            src ++;
            dst += 3;
        }
        src += src_stride - src_size.x;
        dst += dst_stride - src_size.x * 3;
        dst += dst_stride * 2;
    }
}

static void hq4xProcess(const uint32_t* src, uint32_t* dst, sp::Vector2i src_size, int src_stride, HQ2xConfig config)
{
    uint32_t trY = 48;
    uint32_t trU = 7;
    uint32_t trV = 6;
    uint32_t trA = 0;
    
    trY <<= 16;
    trU <<= 8;
    int dst_stride = src_stride * 4;

    int prevline_offset, nextline_offset;
    uint32_t w[9];

    for(int y = 0; y<src_size.y; y++)
    {
        if (y > 0)
            prevline_offset = -src_stride;
        else if (config.out_of_bounds == HQ2xConfig::OutOfBounds::Wrap)
            prevline_offset = src_stride * (src_size.y - 1);
        else
            prevline_offset = 0;
        if (y < src_size.y - 1)
            nextline_offset = src_stride;
        else if (config.out_of_bounds == HQ2xConfig::OutOfBounds::Wrap)
            nextline_offset = -(src_stride * (src_size.y - 1));
        else
            nextline_offset = 0;

        for(int x = 0; x<src_size.x; x++)
        {
            w[1] = src[prevline_offset];
            w[4] = src[0];
            w[7] = src[nextline_offset];

            if (x > 0)
            {
                w[0] = src[prevline_offset - 1];
                w[3] = src[-1];
                w[6] = src[nextline_offset - 1];
            }
            else if (config.out_of_bounds == HQ2xConfig::OutOfBounds::Wrap)
            {
                w[0] = src[prevline_offset + src_size.x - 1];
                w[3] = src[src_size.x - 1];
                w[6] = src[nextline_offset + src_size.x - 1];
            }
            else if (config.out_of_bounds == HQ2xConfig::OutOfBounds::Clamp)
            {
                w[0] = w[1];
                w[3] = w[4];
                w[6] = w[7];
            }
            else if (config.out_of_bounds == HQ2xConfig::OutOfBounds::Transparent)
            {
                w[0] = 0;
                w[3] = 0;
                w[6] = 0;
            }

            if (x < src_size.x - 1)
            {
                w[2] = src[prevline_offset + 1];
                w[5] = src[1];
                w[8] = src[nextline_offset + 1];
            }
            else if (config.out_of_bounds == HQ2xConfig::OutOfBounds::Wrap)
            {
                w[2] = src[prevline_offset - src_size.x + 1];
                w[5] = src[-src_size.x + 1];
                w[8] = src[nextline_offset - src_size.x + 1];
            }
            else if (config.out_of_bounds == HQ2xConfig::OutOfBounds::Clamp)
            {
                w[2] = w[1];
                w[5] = w[4];
                w[8] = w[7];
            }
            else if (config.out_of_bounds == HQ2xConfig::OutOfBounds::Transparent)
            {
                w[2] = 0;
                w[5] = 0;
                w[8] = 0;
            }

            if (y == 0 && config.out_of_bounds == HQ2xConfig::OutOfBounds::Transparent)
                w[0] = w[1] = w[2] = 0;
            if (y == src_size.y - 1 && config.out_of_bounds == HQ2xConfig::OutOfBounds::Transparent)
                w[6] = w[7] = w[8] = 0;

            int pattern = 0;
            int flag = 1;

            for(int k = 0; k < 9; k++)
            {
                if (k == 4) continue;

                if (w[k] != w[4])
                {
                    if (diff(w[4], w[k], trY, trU, trV, trA))
                        pattern |= flag;
                }
                flag <<= 1;
            }

            switch (pattern)
            {
                case 0:
                case 1:
                case 4:
                case 32:
                case 128:
                case 5:
                case 132:
                case 160:
                case 33:
                case 129:
                case 36:
                case 133:
                case 164:
                case 161:
                case 37:
                case 165:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[3]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[5]);
                        dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 2] = mix(6, 1, 1, w[4], w[5], w[1]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[1]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 1] = mix(6, 1, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 2] = mix(6, 1, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[5]);
                        dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        break;
                    }
                case 2:
                case 34:
                case 130:
                case 162:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[2]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 1] = mix(6, 1, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 2] = mix(6, 1, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[5]);
                        dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        break;
                    }
                case 16:
                case 17:
                case 48:
                case 49:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[3]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 1] = mix(6, 1, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 64:
                case 65:
                case 68:
                case 69:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[3]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[5]);
                        dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 2] = mix(6, 1, 1, w[4], w[5], w[1]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[1]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 8:
                case 12:
                case 136:
                case 140:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[0]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[5]);
                        dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(6, 1, 1, w[4], w[5], w[1]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[1]);
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(6, 1, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[6]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[5]);
                        dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        break;
                    }
                case 3:
                case 35:
                case 131:
                case 163:
                    {
                        dst[0] = mix(5, 3, w[4], w[3]);
                        dst[1] = mix(7, 1, w[4], w[3]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[2]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 1] = mix(6, 1, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 2] = mix(6, 1, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[5]);
                        dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        break;
                    }
                case 6:
                case 38:
                case 134:
                case 166:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        dst[2] = mix(7, 1, w[4], w[5]);
                        dst[3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 1] = mix(6, 1, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 2] = mix(6, 1, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[5]);
                        dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        break;
                    }
                case 20:
                case 21:
                case 52:
                case 53:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[3]);
                        dst[2] = mix(5, 3, w[4], w[1]);
                        dst[3] = mix(5, 3, w[4], w[1]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 3] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 1] = mix(6, 1, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 144:
                case 145:
                case 176:
                case 177:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[3]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 1] = mix(6, 1, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 3] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 2] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[7]);
                        break;
                    }
                case 192:
                case 193:
                case 196:
                case 197:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[3]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[5]);
                        dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 2] = mix(6, 1, 1, w[4], w[5], w[1]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[1]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 2 + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 3 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[5]);
                        break;
                    }
                case 96:
                case 97:
                case 100:
                case 101:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[3]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[5]);
                        dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 2] = mix(6, 1, 1, w[4], w[5], w[1]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[1]);
                        dst[dst_stride * 2] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 3 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 40:
                case 44:
                case 168:
                case 172:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[0]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[5]);
                        dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(6, 1, 1, w[4], w[5], w[1]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[1]);
                        dst[dst_stride * 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 2] = mix(6, 1, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 1] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[5]);
                        dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        break;
                    }
                case 9:
                case 13:
                case 137:
                case 141:
                    {
                        dst[0] = mix(5, 3, w[4], w[1]);
                        dst[1] = mix(5, 3, w[4], w[1]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[5]);
                        dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 2] = mix(6, 1, 1, w[4], w[5], w[1]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[1]);
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(6, 1, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[6]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[5]);
                        dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        break;
                    }
                case 18:
                case 50:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                            dst[3] = mix(5, 3, w[4], w[2]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[2] = mix(1, 1, w[1], w[4]);
                            dst[3] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + 3] = mix(1, 1, w[5], w[4]);
                        }
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 1] = mix(6, 1, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 80:
                case 81:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[3]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride * 2 + 2] = w[4];
                            dst[dst_stride * 2 + 3] = mix(1, 1, w[5], w[4]);
                            dst[dst_stride * 3 + 2] = mix(1, 1, w[7], w[4]);
                            dst[dst_stride * 3 + 3] = mix(1, 1, w[7], w[5]);
                        }
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        break;
                    }
                case 72:
                case 76:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[0]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[5]);
                        dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(6, 1, 1, w[4], w[5], w[1]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[1]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                            dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                            dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(1, 1, w[3], w[4]);
                            dst[dst_stride * 2 + 1] = w[4];
                            dst[dst_stride * 3] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(1, 1, w[7], w[4]);
                        }
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[8]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 10:
                case 138:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(5, 3, w[4], w[0]);
                            dst[1] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[1], w[3]);
                            dst[1] = mix(1, 1, w[1], w[4]);
                            dst[dst_stride] = mix(1, 1, w[3], w[4]);
                            dst[dst_stride + 1] = w[4];
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[2]);
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(6, 1, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[6]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[5]);
                        dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        break;
                    }
                case 66:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[2]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 24:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[0]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[6]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 7:
                case 39:
                case 135:
                    {
                        dst[0] = mix(5, 3, w[4], w[3]);
                        dst[1] = mix(7, 1, w[4], w[3]);
                        dst[2] = mix(7, 1, w[4], w[5]);
                        dst[3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 1] = mix(6, 1, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 2] = mix(6, 1, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[5]);
                        dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        break;
                    }
                case 148:
                case 149:
                case 180:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[3]);
                        dst[2] = mix(5, 3, w[4], w[1]);
                        dst[3] = mix(5, 3, w[4], w[1]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 3] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 1] = mix(6, 1, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 3] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 2] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[7]);
                        break;
                    }
                case 224:
                case 228:
                case 225:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[3]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[5]);
                        dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 2] = mix(6, 1, 1, w[4], w[5], w[1]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[1]);
                        dst[dst_stride * 2] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 2 + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 3 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 3 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[5]);
                        break;
                    }
                case 41:
                case 169:
                case 45:
                    {
                        dst[0] = mix(5, 3, w[4], w[1]);
                        dst[1] = mix(5, 3, w[4], w[1]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[5]);
                        dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 2] = mix(6, 1, 1, w[4], w[5], w[1]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[1]);
                        dst[dst_stride * 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 2] = mix(6, 1, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 1] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[5]);
                        dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        break;
                    }
                case 22:
                case 54:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                            dst[3] = w[4];
                            dst[dst_stride + 3] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(1, 1, w[1], w[4]);
                            dst[3] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 3] = mix(1, 1, w[5], w[4]);
                        }
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 1] = mix(6, 1, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 208:
                case 209:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[3]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 3] = w[4];
                            dst[dst_stride * 3 + 2] = w[4];
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2 + 3] = mix(1, 1, w[5], w[4]);
                            dst[dst_stride * 3 + 2] = mix(1, 1, w[7], w[4]);
                            dst[dst_stride * 3 + 3] = mix(1, 1, w[7], w[5]);
                        }
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        break;
                    }
                case 104:
                case 108:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[0]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[5]);
                        dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(6, 1, 1, w[4], w[5], w[1]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[1]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = w[4];
                            dst[dst_stride * 3] = w[4];
                            dst[dst_stride * 3 + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(1, 1, w[3], w[4]);
                            dst[dst_stride * 3] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(1, 1, w[7], w[4]);
                        }
                        dst[dst_stride * 2 + 1] = w[4];
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[8]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 11:
                case 139:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[1], w[3]);
                            dst[1] = mix(1, 1, w[1], w[4]);
                            dst[dst_stride] = mix(1, 1, w[3], w[4]);
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[2]);
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(6, 1, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[6]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[5]);
                        dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        break;
                    }
                case 19:
                case 51:
                    {
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[0] = mix(5, 3, w[4], w[3]);
                            dst[1] = mix(7, 1, w[4], w[3]);
                            dst[2] = mix(3, 1, w[4], w[2]);
                            dst[3] = mix(5, 3, w[4], w[2]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[0] = mix(3, 1, w[4], w[1]);
                            dst[1] = mix(3, 1, w[1], w[4]);
                            dst[2] = mix(5, 3, w[1], w[5]);
                            dst[3] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 2] = mix(6, 1, 1, w[4], w[5], w[1]);
                            dst[dst_stride + 3] = mix(2, 1, 1, w[5], w[4], w[1]);
                        }
                        dst[dst_stride] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 1] = mix(6, 1, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 146:
                case 178:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                            dst[3] = mix(5, 3, w[4], w[2]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                            dst[dst_stride * 2 + 3] = mix(7, 1, w[4], w[7]);
                            dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[7]);
                        }
                        else
                        {
                            dst[2] = mix(2, 1, 1, w[1], w[4], w[5]);
                            dst[3] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 2] = mix(6, 1, 1, w[4], w[5], w[1]);
                            dst[dst_stride + 3] = mix(5, 3, w[5], w[1]);
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[5], w[4]);
                            dst[dst_stride * 3 + 3] = mix(3, 1, w[4], w[5]);
                        }
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 1] = mix(6, 1, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 2] = mix(5, 3, w[4], w[7]);
                        break;
                    }
                case 84:
                case 85:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[3]);
                        dst[2] = mix(5, 3, w[4], w[1]);
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[3] = mix(5, 3, w[4], w[1]);
                            dst[dst_stride + 3] = mix(7, 1, w[4], w[1]);
                            dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        }
                        else
                        {
                            dst[3] = mix(3, 1, w[4], w[5]);
                            dst[dst_stride + 3] = mix(3, 1, w[5], w[4]);
                            dst[dst_stride * 2 + 2] = mix(6, 1, 1, w[4], w[5], w[7]);
                            dst[dst_stride * 2 + 3] = mix(5, 3, w[5], w[7]);
                            dst[dst_stride * 3 + 2] = mix(2, 1, 1, w[7], w[4], w[5]);
                            dst[dst_stride * 3 + 3] = mix(1, 1, w[7], w[5]);
                        }
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        break;
                    }
                case 112:
                case 113:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[3]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride * 2] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[3]);
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3] = mix(5, 3, w[4], w[3]);
                            dst[dst_stride * 3 + 1] = mix(7, 1, w[4], w[3]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride * 2 + 2] = mix(6, 1, 1, w[4], w[5], w[7]);
                            dst[dst_stride * 2 + 3] = mix(2, 1, 1, w[5], w[4], w[7]);
                            dst[dst_stride * 3] = mix(3, 1, w[4], w[7]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[7], w[4]);
                            dst[dst_stride * 3 + 2] = mix(5, 3, w[7], w[5]);
                            dst[dst_stride * 3 + 3] = mix(1, 1, w[7], w[5]);
                        }
                        break;
                    }
                case 200:
                case 204:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[0]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[5]);
                        dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(6, 1, 1, w[4], w[5], w[1]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[1]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                            dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                            dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                            dst[dst_stride * 3 + 2] = mix(7, 1, w[4], w[5]);
                            dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[5]);
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(2, 1, 1, w[3], w[4], w[7]);
                            dst[dst_stride * 2 + 1] = mix(6, 1, 1, w[4], w[3], w[7]);
                            dst[dst_stride * 3] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(5, 3, w[7], w[3]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[7], w[4]);
                            dst[dst_stride * 3 + 3] = mix(3, 1, w[4], w[7]);
                        }
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 2 + 3] = mix(5, 3, w[4], w[5]);
                        break;
                    }
                case 73:
                case 77:
                    {
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[0] = mix(5, 3, w[4], w[1]);
                            dst[dst_stride] = mix(7, 1, w[4], w[1]);
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                            dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                            dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[0] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride] = mix(3, 1, w[3], w[4]);
                            dst[dst_stride * 2] = mix(5, 3, w[3], w[7]);
                            dst[dst_stride * 2 + 1] = mix(6, 1, 1, w[4], w[3], w[7]);
                            dst[dst_stride * 3] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(2, 1, 1, w[7], w[4], w[3]);
                        }
                        dst[1] = mix(5, 3, w[4], w[1]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[5]);
                        dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 2] = mix(6, 1, 1, w[4], w[5], w[1]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[1]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[8]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 42:
                case 170:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(5, 3, w[4], w[0]);
                            dst[1] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                            dst[dst_stride * 2] = mix(7, 1, w[4], w[7]);
                            dst[dst_stride * 3] = mix(5, 3, w[4], w[7]);
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[1], w[3]);
                            dst[1] = mix(2, 1, 1, w[1], w[4], w[3]);
                            dst[dst_stride] = mix(5, 3, w[3], w[1]);
                            dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[3], w[1]);
                            dst[dst_stride * 2] = mix(3, 1, w[3], w[4]);
                            dst[dst_stride * 3] = mix(3, 1, w[4], w[3]);
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[2]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 2] = mix(6, 1, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 3 + 1] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[5]);
                        dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        break;
                    }
                case 14:
                case 142:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(5, 3, w[4], w[0]);
                            dst[1] = mix(3, 1, w[4], w[0]);
                            dst[2] = mix(7, 1, w[4], w[5]);
                            dst[3] = mix(5, 3, w[4], w[5]);
                            dst[dst_stride] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[1], w[3]);
                            dst[1] = mix(5, 3, w[1], w[3]);
                            dst[2] = mix(3, 1, w[1], w[4]);
                            dst[3] = mix(3, 1, w[4], w[1]);
                            dst[dst_stride] = mix(2, 1, 1, w[3], w[4], w[1]);
                            dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[3], w[1]);
                        }
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(6, 1, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[6]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[5]);
                        dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        break;
                    }
                case 67:
                    {
                        dst[0] = mix(5, 3, w[4], w[3]);
                        dst[1] = mix(7, 1, w[4], w[3]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[2]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 70:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        dst[2] = mix(7, 1, w[4], w[5]);
                        dst[3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 28:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[0]);
                        dst[2] = mix(5, 3, w[4], w[1]);
                        dst[3] = mix(5, 3, w[4], w[1]);
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 3] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[6]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 152:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[0]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 3] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[6]);
                        dst[dst_stride * 3 + 2] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[7]);
                        break;
                    }
                case 194:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[2]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 2 + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 3 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[5]);
                        break;
                    }
                case 98:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[2]);
                        dst[dst_stride * 2] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 3 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 56:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[0]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride * 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 1] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 25:
                    {
                        dst[0] = mix(5, 3, w[4], w[1]);
                        dst[1] = mix(5, 3, w[4], w[1]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[6]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 26:
                case 31:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[1], w[3]);
                            dst[1] = mix(1, 1, w[1], w[4]);
                            dst[dst_stride] = mix(1, 1, w[3], w[4]);
                        }
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                            dst[3] = w[4];
                            dst[dst_stride + 3] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(1, 1, w[1], w[4]);
                            dst[3] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 3] = mix(1, 1, w[5], w[4]);
                        }
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[6]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 82:
                case 214:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                            dst[3] = w[4];
                            dst[dst_stride + 3] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(1, 1, w[1], w[4]);
                            dst[3] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 3] = mix(1, 1, w[5], w[4]);
                        }
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 3] = w[4];
                            dst[dst_stride * 3 + 2] = w[4];
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2 + 3] = mix(1, 1, w[5], w[4]);
                            dst[dst_stride * 3 + 2] = mix(1, 1, w[7], w[4]);
                            dst[dst_stride * 3 + 3] = mix(1, 1, w[7], w[5]);
                        }
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        break;
                    }
                case 88:
                case 248:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[0]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = w[4];
                            dst[dst_stride * 3] = w[4];
                            dst[dst_stride * 3 + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(1, 1, w[3], w[4]);
                            dst[dst_stride * 3] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(1, 1, w[7], w[4]);
                        }
                        dst[dst_stride * 2 + 1] = w[4];
                        dst[dst_stride * 2 + 2] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 3] = w[4];
                            dst[dst_stride * 3 + 2] = w[4];
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2 + 3] = mix(1, 1, w[5], w[4]);
                            dst[dst_stride * 3 + 2] = mix(1, 1, w[7], w[4]);
                            dst[dst_stride * 3 + 3] = mix(1, 1, w[7], w[5]);
                        }
                        break;
                    }
                case 74:
                case 107:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[1], w[3]);
                            dst[1] = mix(1, 1, w[1], w[4]);
                            dst[dst_stride] = mix(1, 1, w[3], w[4]);
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[2]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = w[4];
                            dst[dst_stride * 3] = w[4];
                            dst[dst_stride * 3 + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(1, 1, w[3], w[4]);
                            dst[dst_stride * 3] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(1, 1, w[7], w[4]);
                        }
                        dst[dst_stride * 2 + 1] = w[4];
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[8]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 27:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[1], w[3]);
                            dst[1] = mix(1, 1, w[1], w[4]);
                            dst[dst_stride] = mix(1, 1, w[3], w[4]);
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[6]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 86:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                            dst[3] = w[4];
                            dst[dst_stride + 3] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(1, 1, w[1], w[4]);
                            dst[3] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 3] = mix(1, 1, w[5], w[4]);
                        }
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 216:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[0]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 3] = w[4];
                            dst[dst_stride * 3 + 2] = w[4];
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2 + 3] = mix(1, 1, w[5], w[4]);
                            dst[dst_stride * 3 + 2] = mix(1, 1, w[7], w[4]);
                            dst[dst_stride * 3 + 3] = mix(1, 1, w[7], w[5]);
                        }
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        break;
                    }
                case 106:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[2]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = w[4];
                            dst[dst_stride * 3] = w[4];
                            dst[dst_stride * 3 + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(1, 1, w[3], w[4]);
                            dst[dst_stride * 3] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(1, 1, w[7], w[4]);
                        }
                        dst[dst_stride * 2 + 1] = w[4];
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[8]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 30:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                            dst[3] = w[4];
                            dst[dst_stride + 3] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(1, 1, w[1], w[4]);
                            dst[3] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 3] = mix(1, 1, w[5], w[4]);
                        }
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[6]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 210:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 3] = w[4];
                            dst[dst_stride * 3 + 2] = w[4];
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2 + 3] = mix(1, 1, w[5], w[4]);
                            dst[dst_stride * 3 + 2] = mix(1, 1, w[7], w[4]);
                            dst[dst_stride * 3 + 3] = mix(1, 1, w[7], w[5]);
                        }
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        break;
                    }
                case 120:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[0]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = w[4];
                            dst[dst_stride * 3] = w[4];
                            dst[dst_stride * 3 + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(1, 1, w[3], w[4]);
                            dst[dst_stride * 3] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(1, 1, w[7], w[4]);
                        }
                        dst[dst_stride * 2 + 1] = w[4];
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 75:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[1], w[3]);
                            dst[1] = mix(1, 1, w[1], w[4]);
                            dst[dst_stride] = mix(1, 1, w[3], w[4]);
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[2]);
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 29:
                    {
                        dst[0] = mix(5, 3, w[4], w[1]);
                        dst[1] = mix(5, 3, w[4], w[1]);
                        dst[2] = mix(5, 3, w[4], w[1]);
                        dst[3] = mix(5, 3, w[4], w[1]);
                        dst[dst_stride] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 3] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[6]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 198:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        dst[2] = mix(7, 1, w[4], w[5]);
                        dst[3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 2 + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 3 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[5]);
                        break;
                    }
                case 184:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[0]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride * 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 3] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 1] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 2] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[7]);
                        break;
                    }
                case 99:
                    {
                        dst[0] = mix(5, 3, w[4], w[3]);
                        dst[1] = mix(7, 1, w[4], w[3]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[2]);
                        dst[dst_stride * 2] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 3 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 57:
                    {
                        dst[0] = mix(5, 3, w[4], w[1]);
                        dst[1] = mix(5, 3, w[4], w[1]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride * 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 1] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 71:
                    {
                        dst[0] = mix(5, 3, w[4], w[3]);
                        dst[1] = mix(7, 1, w[4], w[3]);
                        dst[2] = mix(7, 1, w[4], w[5]);
                        dst[3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 156:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[0]);
                        dst[2] = mix(5, 3, w[4], w[1]);
                        dst[3] = mix(5, 3, w[4], w[1]);
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 3] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 3] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[6]);
                        dst[dst_stride * 3 + 2] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[7]);
                        break;
                    }
                case 226:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[2]);
                        dst[dst_stride * 2] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 2 + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 3 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 3 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[5]);
                        break;
                    }
                case 60:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[0]);
                        dst[2] = mix(5, 3, w[4], w[1]);
                        dst[3] = mix(5, 3, w[4], w[1]);
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 3] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride * 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 1] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 195:
                    {
                        dst[0] = mix(5, 3, w[4], w[3]);
                        dst[1] = mix(7, 1, w[4], w[3]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[2]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 2 + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 3 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[5]);
                        break;
                    }
                case 102:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        dst[2] = mix(7, 1, w[4], w[5]);
                        dst[3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 2] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 3 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 153:
                    {
                        dst[0] = mix(5, 3, w[4], w[1]);
                        dst[1] = mix(5, 3, w[4], w[1]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 3] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[6]);
                        dst[dst_stride * 3 + 2] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[7]);
                        break;
                    }
                case 58:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(5, 3, w[4], w[0]);
                            dst[1] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                            dst[1] = mix(3, 1, w[4], w[1]);
                            dst[dst_stride] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride + 1] = w[4];
                        }
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                            dst[3] = mix(5, 3, w[4], w[2]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[2] = mix(3, 1, w[4], w[1]);
                            dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[5]);
                        }
                        dst[dst_stride * 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 1] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 83:
                    {
                        dst[0] = mix(5, 3, w[4], w[3]);
                        dst[1] = mix(7, 1, w[4], w[3]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                            dst[3] = mix(5, 3, w[4], w[2]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[2] = mix(3, 1, w[4], w[1]);
                            dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[5]);
                        }
                        dst[dst_stride] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride * 2 + 2] = w[4];
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[5]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[7]);
                            dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        }
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        break;
                    }
                case 92:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[0]);
                        dst[2] = mix(5, 3, w[4], w[1]);
                        dst[3] = mix(5, 3, w[4], w[1]);
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 3] = mix(7, 1, w[4], w[1]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                            dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                            dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride * 2 + 1] = w[4];
                            dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[7]);
                        }
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride * 2 + 2] = w[4];
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[5]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[7]);
                            dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        }
                        break;
                    }
                case 202:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(5, 3, w[4], w[0]);
                            dst[1] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                            dst[1] = mix(3, 1, w[4], w[1]);
                            dst[dst_stride] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride + 1] = w[4];
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[2]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                            dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                            dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride * 2 + 1] = w[4];
                            dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[7]);
                        }
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 2 + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 3 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[5]);
                        break;
                    }
                case 78:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(5, 3, w[4], w[0]);
                            dst[1] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                            dst[1] = mix(3, 1, w[4], w[1]);
                            dst[dst_stride] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride + 1] = w[4];
                        }
                        dst[2] = mix(7, 1, w[4], w[5]);
                        dst[3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride + 3] = mix(5, 3, w[4], w[5]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                            dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                            dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride * 2 + 1] = w[4];
                            dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[7]);
                        }
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[8]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 154:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(5, 3, w[4], w[0]);
                            dst[1] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                            dst[1] = mix(3, 1, w[4], w[1]);
                            dst[dst_stride] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride + 1] = w[4];
                        }
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                            dst[3] = mix(5, 3, w[4], w[2]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[2] = mix(3, 1, w[4], w[1]);
                            dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[5]);
                        }
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 3] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[6]);
                        dst[dst_stride * 3 + 2] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[7]);
                        break;
                    }
                case 114:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                            dst[3] = mix(5, 3, w[4], w[2]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[2] = mix(3, 1, w[4], w[1]);
                            dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[5]);
                        }
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride * 2] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[3]);
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride * 2 + 2] = w[4];
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[5]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[7]);
                            dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        }
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 3 + 1] = mix(7, 1, w[4], w[3]);
                        break;
                    }
                case 89:
                    {
                        dst[0] = mix(5, 3, w[4], w[1]);
                        dst[1] = mix(5, 3, w[4], w[1]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                            dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                            dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride * 2 + 1] = w[4];
                            dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[7]);
                        }
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride * 2 + 2] = w[4];
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[5]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[7]);
                            dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        }
                        break;
                    }
                case 90:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(5, 3, w[4], w[0]);
                            dst[1] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                            dst[1] = mix(3, 1, w[4], w[1]);
                            dst[dst_stride] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride + 1] = w[4];
                        }
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                            dst[3] = mix(5, 3, w[4], w[2]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[2] = mix(3, 1, w[4], w[1]);
                            dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[5]);
                        }
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                            dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                            dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride * 2 + 1] = w[4];
                            dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[7]);
                        }
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride * 2 + 2] = w[4];
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[5]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[7]);
                            dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        }
                        break;
                    }
                case 55:
                case 23:
                    {
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[0] = mix(5, 3, w[4], w[3]);
                            dst[1] = mix(7, 1, w[4], w[3]);
                            dst[2] = w[4];
                            dst[3] = w[4];
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + 3] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(3, 1, w[4], w[1]);
                            dst[1] = mix(3, 1, w[1], w[4]);
                            dst[2] = mix(5, 3, w[1], w[5]);
                            dst[3] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 2] = mix(6, 1, 1, w[4], w[5], w[1]);
                            dst[dst_stride + 3] = mix(2, 1, 1, w[5], w[4], w[1]);
                        }
                        dst[dst_stride] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 1] = mix(6, 1, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 182:
                case 150:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                            dst[3] = w[4];
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + 3] = w[4];
                            dst[dst_stride * 2 + 3] = mix(7, 1, w[4], w[7]);
                            dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[7]);
                        }
                        else
                        {
                            dst[2] = mix(2, 1, 1, w[1], w[4], w[5]);
                            dst[3] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 2] = mix(6, 1, 1, w[4], w[5], w[1]);
                            dst[dst_stride + 3] = mix(5, 3, w[5], w[1]);
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[5], w[4]);
                            dst[dst_stride * 3 + 3] = mix(3, 1, w[4], w[5]);
                        }
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 1] = mix(6, 1, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 2] = mix(5, 3, w[4], w[7]);
                        break;
                    }
                case 213:
                case 212:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[3]);
                        dst[2] = mix(5, 3, w[4], w[1]);
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[3] = mix(5, 3, w[4], w[1]);
                            dst[dst_stride + 3] = mix(7, 1, w[4], w[1]);
                            dst[dst_stride * 2 + 2] = w[4];
                            dst[dst_stride * 2 + 3] = w[4];
                            dst[dst_stride * 3 + 2] = w[4];
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[3] = mix(3, 1, w[4], w[5]);
                            dst[dst_stride + 3] = mix(3, 1, w[5], w[4]);
                            dst[dst_stride * 2 + 2] = mix(6, 1, 1, w[4], w[5], w[7]);
                            dst[dst_stride * 2 + 3] = mix(5, 3, w[5], w[7]);
                            dst[dst_stride * 3 + 2] = mix(2, 1, 1, w[7], w[4], w[5]);
                            dst[dst_stride * 3 + 3] = mix(1, 1, w[7], w[5]);
                        }
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        break;
                    }
                case 241:
                case 240:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[3]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride * 2] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[3]);
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 2] = w[4];
                            dst[dst_stride * 2 + 3] = w[4];
                            dst[dst_stride * 3] = mix(5, 3, w[4], w[3]);
                            dst[dst_stride * 3 + 1] = mix(7, 1, w[4], w[3]);
                            dst[dst_stride * 3 + 2] = w[4];
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2 + 2] = mix(6, 1, 1, w[4], w[5], w[7]);
                            dst[dst_stride * 2 + 3] = mix(2, 1, 1, w[5], w[4], w[7]);
                            dst[dst_stride * 3] = mix(3, 1, w[4], w[7]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[7], w[4]);
                            dst[dst_stride * 3 + 2] = mix(5, 3, w[7], w[5]);
                            dst[dst_stride * 3 + 3] = mix(1, 1, w[7], w[5]);
                        }
                        break;
                    }
                case 236:
                case 232:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[0]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[5]);
                        dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(6, 1, 1, w[4], w[5], w[1]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[1]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = w[4];
                            dst[dst_stride * 2 + 1] = w[4];
                            dst[dst_stride * 3] = w[4];
                            dst[dst_stride * 3 + 1] = w[4];
                            dst[dst_stride * 3 + 2] = mix(7, 1, w[4], w[5]);
                            dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[5]);
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(2, 1, 1, w[3], w[4], w[7]);
                            dst[dst_stride * 2 + 1] = mix(6, 1, 1, w[4], w[3], w[7]);
                            dst[dst_stride * 3] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(5, 3, w[7], w[3]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[7], w[4]);
                            dst[dst_stride * 3 + 3] = mix(3, 1, w[4], w[7]);
                        }
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 2 + 3] = mix(5, 3, w[4], w[5]);
                        break;
                    }
                case 109:
                case 105:
                    {
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[0] = mix(5, 3, w[4], w[1]);
                            dst[dst_stride] = mix(7, 1, w[4], w[1]);
                            dst[dst_stride * 2] = w[4];
                            dst[dst_stride * 2 + 1] = w[4];
                            dst[dst_stride * 3] = w[4];
                            dst[dst_stride * 3 + 1] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride] = mix(3, 1, w[3], w[4]);
                            dst[dst_stride * 2] = mix(5, 3, w[3], w[7]);
                            dst[dst_stride * 2 + 1] = mix(6, 1, 1, w[4], w[3], w[7]);
                            dst[dst_stride * 3] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(2, 1, 1, w[7], w[4], w[3]);
                        }
                        dst[1] = mix(5, 3, w[4], w[1]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[5]);
                        dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 2] = mix(6, 1, 1, w[4], w[5], w[1]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[1]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[8]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 171:
                case 43:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                            dst[dst_stride + 1] = w[4];
                            dst[dst_stride * 2] = mix(7, 1, w[4], w[7]);
                            dst[dst_stride * 3] = mix(5, 3, w[4], w[7]);
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[1], w[3]);
                            dst[1] = mix(2, 1, 1, w[1], w[4], w[3]);
                            dst[dst_stride] = mix(5, 3, w[3], w[1]);
                            dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[3], w[1]);
                            dst[dst_stride * 2] = mix(3, 1, w[3], w[4]);
                            dst[dst_stride * 3] = mix(3, 1, w[4], w[3]);
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[2]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 2] = mix(6, 1, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 3 + 1] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[5]);
                        dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        break;
                    }
                case 143:
                case 15:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[2] = mix(7, 1, w[4], w[5]);
                            dst[3] = mix(5, 3, w[4], w[5]);
                            dst[dst_stride] = w[4];
                            dst[dst_stride + 1] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[1], w[3]);
                            dst[1] = mix(5, 3, w[1], w[3]);
                            dst[2] = mix(3, 1, w[1], w[4]);
                            dst[3] = mix(3, 1, w[4], w[1]);
                            dst[dst_stride] = mix(2, 1, 1, w[3], w[4], w[1]);
                            dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[3], w[1]);
                        }
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(6, 1, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[6]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[5]);
                        dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        break;
                    }
                case 124:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[0]);
                        dst[2] = mix(5, 3, w[4], w[1]);
                        dst[3] = mix(5, 3, w[4], w[1]);
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 3] = mix(7, 1, w[4], w[1]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = w[4];
                            dst[dst_stride * 3] = w[4];
                            dst[dst_stride * 3 + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(1, 1, w[3], w[4]);
                            dst[dst_stride * 3] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(1, 1, w[7], w[4]);
                        }
                        dst[dst_stride * 2 + 1] = w[4];
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 203:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[1], w[3]);
                            dst[1] = mix(1, 1, w[1], w[4]);
                            dst[dst_stride] = mix(1, 1, w[3], w[4]);
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[2]);
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 2 + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 3 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[5]);
                        break;
                    }
                case 62:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                            dst[3] = w[4];
                            dst[dst_stride + 3] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(1, 1, w[1], w[4]);
                            dst[3] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 3] = mix(1, 1, w[5], w[4]);
                        }
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride * 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 1] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 211:
                    {
                        dst[0] = mix(5, 3, w[4], w[3]);
                        dst[1] = mix(7, 1, w[4], w[3]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 3] = w[4];
                            dst[dst_stride * 3 + 2] = w[4];
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2 + 3] = mix(1, 1, w[5], w[4]);
                            dst[dst_stride * 3 + 2] = mix(1, 1, w[7], w[4]);
                            dst[dst_stride * 3 + 3] = mix(1, 1, w[7], w[5]);
                        }
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        break;
                    }
                case 118:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                            dst[3] = w[4];
                            dst[dst_stride + 3] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(1, 1, w[1], w[4]);
                            dst[3] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 3] = mix(1, 1, w[5], w[4]);
                        }
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride * 2] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 3 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 217:
                    {
                        dst[0] = mix(5, 3, w[4], w[1]);
                        dst[1] = mix(5, 3, w[4], w[1]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 3] = w[4];
                            dst[dst_stride * 3 + 2] = w[4];
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2 + 3] = mix(1, 1, w[5], w[4]);
                            dst[dst_stride * 3 + 2] = mix(1, 1, w[7], w[4]);
                            dst[dst_stride * 3 + 3] = mix(1, 1, w[7], w[5]);
                        }
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        break;
                    }
                case 110:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        dst[2] = mix(7, 1, w[4], w[5]);
                        dst[3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride + 3] = mix(5, 3, w[4], w[5]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = w[4];
                            dst[dst_stride * 3] = w[4];
                            dst[dst_stride * 3 + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(1, 1, w[3], w[4]);
                            dst[dst_stride * 3] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(1, 1, w[7], w[4]);
                        }
                        dst[dst_stride * 2 + 1] = w[4];
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[8]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 155:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[1], w[3]);
                            dst[1] = mix(1, 1, w[1], w[4]);
                            dst[dst_stride] = mix(1, 1, w[3], w[4]);
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 3] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[6]);
                        dst[dst_stride * 3 + 2] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[7]);
                        break;
                    }
                case 188:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[0]);
                        dst[2] = mix(5, 3, w[4], w[1]);
                        dst[3] = mix(5, 3, w[4], w[1]);
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 3] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride * 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 3] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 1] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 2] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[7]);
                        break;
                    }
                case 185:
                    {
                        dst[0] = mix(5, 3, w[4], w[1]);
                        dst[1] = mix(5, 3, w[4], w[1]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride * 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 3] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 1] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 2] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[7]);
                        break;
                    }
                case 61:
                    {
                        dst[0] = mix(5, 3, w[4], w[1]);
                        dst[1] = mix(5, 3, w[4], w[1]);
                        dst[2] = mix(5, 3, w[4], w[1]);
                        dst[3] = mix(5, 3, w[4], w[1]);
                        dst[dst_stride] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 3] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride * 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 1] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 157:
                    {
                        dst[0] = mix(5, 3, w[4], w[1]);
                        dst[1] = mix(5, 3, w[4], w[1]);
                        dst[2] = mix(5, 3, w[4], w[1]);
                        dst[3] = mix(5, 3, w[4], w[1]);
                        dst[dst_stride] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 3] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 3] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[6]);
                        dst[dst_stride * 3 + 2] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[7]);
                        break;
                    }
                case 103:
                    {
                        dst[0] = mix(5, 3, w[4], w[3]);
                        dst[1] = mix(7, 1, w[4], w[3]);
                        dst[2] = mix(7, 1, w[4], w[5]);
                        dst[3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 2] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 3 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 227:
                    {
                        dst[0] = mix(5, 3, w[4], w[3]);
                        dst[1] = mix(7, 1, w[4], w[3]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[2]);
                        dst[dst_stride * 2] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 2 + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 3 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 3 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[5]);
                        break;
                    }
                case 230:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        dst[2] = mix(7, 1, w[4], w[5]);
                        dst[3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 2] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 2 + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 3 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 3 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[5]);
                        break;
                    }
                case 199:
                    {
                        dst[0] = mix(5, 3, w[4], w[3]);
                        dst[1] = mix(7, 1, w[4], w[3]);
                        dst[2] = mix(7, 1, w[4], w[5]);
                        dst[3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 2 + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 3 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[5]);
                        break;
                    }
                case 220:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[0]);
                        dst[2] = mix(5, 3, w[4], w[1]);
                        dst[3] = mix(5, 3, w[4], w[1]);
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 3] = mix(7, 1, w[4], w[1]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                            dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                            dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride * 2 + 1] = w[4];
                            dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[7]);
                        }
                        dst[dst_stride * 2 + 2] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 3] = w[4];
                            dst[dst_stride * 3 + 2] = w[4];
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2 + 3] = mix(1, 1, w[5], w[4]);
                            dst[dst_stride * 3 + 2] = mix(1, 1, w[7], w[4]);
                            dst[dst_stride * 3 + 3] = mix(1, 1, w[7], w[5]);
                        }
                        break;
                    }
                case 158:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(5, 3, w[4], w[0]);
                            dst[1] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                            dst[1] = mix(3, 1, w[4], w[1]);
                            dst[dst_stride] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride + 1] = w[4];
                        }
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                            dst[3] = w[4];
                            dst[dst_stride + 3] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(1, 1, w[1], w[4]);
                            dst[3] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 3] = mix(1, 1, w[5], w[4]);
                        }
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 3] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[6]);
                        dst[dst_stride * 3 + 2] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[7]);
                        break;
                    }
                case 234:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(5, 3, w[4], w[0]);
                            dst[1] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                            dst[1] = mix(3, 1, w[4], w[1]);
                            dst[dst_stride] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride + 1] = w[4];
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[2]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = w[4];
                            dst[dst_stride * 3] = w[4];
                            dst[dst_stride * 3 + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(1, 1, w[3], w[4]);
                            dst[dst_stride * 3] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(1, 1, w[7], w[4]);
                        }
                        dst[dst_stride * 2 + 1] = w[4];
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 2 + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 3 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[5]);
                        break;
                    }
                case 242:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                            dst[3] = mix(5, 3, w[4], w[2]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[2] = mix(3, 1, w[4], w[1]);
                            dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[5]);
                        }
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride * 2] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 2 + 2] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 3] = w[4];
                            dst[dst_stride * 3 + 2] = w[4];
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2 + 3] = mix(1, 1, w[5], w[4]);
                            dst[dst_stride * 3 + 2] = mix(1, 1, w[7], w[4]);
                            dst[dst_stride * 3 + 3] = mix(1, 1, w[7], w[5]);
                        }
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 3 + 1] = mix(7, 1, w[4], w[3]);
                        break;
                    }
                case 59:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[1], w[3]);
                            dst[1] = mix(1, 1, w[1], w[4]);
                            dst[dst_stride] = mix(1, 1, w[3], w[4]);
                        }
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                            dst[3] = mix(5, 3, w[4], w[2]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[2] = mix(3, 1, w[4], w[1]);
                            dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[5]);
                        }
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride * 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 1] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 121:
                    {
                        dst[0] = mix(5, 3, w[4], w[1]);
                        dst[1] = mix(5, 3, w[4], w[1]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = w[4];
                            dst[dst_stride * 3] = w[4];
                            dst[dst_stride * 3 + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(1, 1, w[3], w[4]);
                            dst[dst_stride * 3] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(1, 1, w[7], w[4]);
                        }
                        dst[dst_stride * 2 + 1] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride * 2 + 2] = w[4];
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[5]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[7]);
                            dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        }
                        break;
                    }
                case 87:
                    {
                        dst[0] = mix(5, 3, w[4], w[3]);
                        dst[1] = mix(7, 1, w[4], w[3]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                            dst[3] = w[4];
                            dst[dst_stride + 3] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(1, 1, w[1], w[4]);
                            dst[3] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 3] = mix(1, 1, w[5], w[4]);
                        }
                        dst[dst_stride] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride * 2 + 2] = w[4];
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[5]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[7]);
                            dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        }
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        break;
                    }
                case 79:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[1], w[3]);
                            dst[1] = mix(1, 1, w[1], w[4]);
                            dst[dst_stride] = mix(1, 1, w[3], w[4]);
                        }
                        dst[2] = mix(7, 1, w[4], w[5]);
                        dst[3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride + 3] = mix(5, 3, w[4], w[5]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                            dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                            dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride * 2 + 1] = w[4];
                            dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[7]);
                        }
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[8]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 122:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(5, 3, w[4], w[0]);
                            dst[1] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                            dst[1] = mix(3, 1, w[4], w[1]);
                            dst[dst_stride] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride + 1] = w[4];
                        }
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                            dst[3] = mix(5, 3, w[4], w[2]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[2] = mix(3, 1, w[4], w[1]);
                            dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[5]);
                        }
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = w[4];
                            dst[dst_stride * 3] = w[4];
                            dst[dst_stride * 3 + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(1, 1, w[3], w[4]);
                            dst[dst_stride * 3] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(1, 1, w[7], w[4]);
                        }
                        dst[dst_stride * 2 + 1] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride * 2 + 2] = w[4];
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[5]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[7]);
                            dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        }
                        break;
                    }
                case 94:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(5, 3, w[4], w[0]);
                            dst[1] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                            dst[1] = mix(3, 1, w[4], w[1]);
                            dst[dst_stride] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride + 1] = w[4];
                        }
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                            dst[3] = w[4];
                            dst[dst_stride + 3] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(1, 1, w[1], w[4]);
                            dst[3] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 3] = mix(1, 1, w[5], w[4]);
                        }
                        dst[dst_stride + 2] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                            dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                            dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride * 2 + 1] = w[4];
                            dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[7]);
                        }
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride * 2 + 2] = w[4];
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[5]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[7]);
                            dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        }
                        break;
                    }
                case 218:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(5, 3, w[4], w[0]);
                            dst[1] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                            dst[1] = mix(3, 1, w[4], w[1]);
                            dst[dst_stride] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride + 1] = w[4];
                        }
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                            dst[3] = mix(5, 3, w[4], w[2]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[2] = mix(3, 1, w[4], w[1]);
                            dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[5]);
                        }
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                            dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                            dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride * 2 + 1] = w[4];
                            dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[7]);
                        }
                        dst[dst_stride * 2 + 2] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 3] = w[4];
                            dst[dst_stride * 3 + 2] = w[4];
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2 + 3] = mix(1, 1, w[5], w[4]);
                            dst[dst_stride * 3 + 2] = mix(1, 1, w[7], w[4]);
                            dst[dst_stride * 3 + 3] = mix(1, 1, w[7], w[5]);
                        }
                        break;
                    }
                case 91:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[1], w[3]);
                            dst[1] = mix(1, 1, w[1], w[4]);
                            dst[dst_stride] = mix(1, 1, w[3], w[4]);
                        }
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                            dst[3] = mix(5, 3, w[4], w[2]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[2] = mix(3, 1, w[4], w[1]);
                            dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[5]);
                        }
                        dst[dst_stride + 1] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                            dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                            dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride * 2 + 1] = w[4];
                            dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[7]);
                        }
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride * 2 + 2] = w[4];
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[5]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[7]);
                            dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        }
                        break;
                    }
                case 229:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[3]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[5]);
                        dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 2] = mix(6, 1, 1, w[4], w[5], w[1]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[1]);
                        dst[dst_stride * 2] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 2 + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 3 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 3 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[5]);
                        break;
                    }
                case 167:
                    {
                        dst[0] = mix(5, 3, w[4], w[3]);
                        dst[1] = mix(7, 1, w[4], w[3]);
                        dst[2] = mix(7, 1, w[4], w[5]);
                        dst[3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 1] = mix(6, 1, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 2] = mix(6, 1, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[5]);
                        dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        break;
                    }
                case 173:
                    {
                        dst[0] = mix(5, 3, w[4], w[1]);
                        dst[1] = mix(5, 3, w[4], w[1]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[5]);
                        dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 2] = mix(6, 1, 1, w[4], w[5], w[1]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[1]);
                        dst[dst_stride * 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 2] = mix(6, 1, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 1] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[5]);
                        dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        break;
                    }
                case 181:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[3]);
                        dst[2] = mix(5, 3, w[4], w[1]);
                        dst[3] = mix(5, 3, w[4], w[1]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 3] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 1] = mix(6, 1, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 3] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 2] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[7]);
                        break;
                    }
                case 186:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(5, 3, w[4], w[0]);
                            dst[1] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                            dst[1] = mix(3, 1, w[4], w[1]);
                            dst[dst_stride] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride + 1] = w[4];
                        }
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                            dst[3] = mix(5, 3, w[4], w[2]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[2] = mix(3, 1, w[4], w[1]);
                            dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[5]);
                        }
                        dst[dst_stride * 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 3] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 1] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 2] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[7]);
                        break;
                    }
                case 115:
                    {
                        dst[0] = mix(5, 3, w[4], w[3]);
                        dst[1] = mix(7, 1, w[4], w[3]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                            dst[3] = mix(5, 3, w[4], w[2]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[2] = mix(3, 1, w[4], w[1]);
                            dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[5]);
                        }
                        dst[dst_stride] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 2] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[3]);
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride * 2 + 2] = w[4];
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[5]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[7]);
                            dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        }
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 3 + 1] = mix(7, 1, w[4], w[3]);
                        break;
                    }
                case 93:
                    {
                        dst[0] = mix(5, 3, w[4], w[1]);
                        dst[1] = mix(5, 3, w[4], w[1]);
                        dst[2] = mix(5, 3, w[4], w[1]);
                        dst[3] = mix(5, 3, w[4], w[1]);
                        dst[dst_stride] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 3] = mix(7, 1, w[4], w[1]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                            dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                            dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride * 2 + 1] = w[4];
                            dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[7]);
                        }
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride * 2 + 2] = w[4];
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[5]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[7]);
                            dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        }
                        break;
                    }
                case 206:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(5, 3, w[4], w[0]);
                            dst[1] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                            dst[1] = mix(3, 1, w[4], w[1]);
                            dst[dst_stride] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride + 1] = w[4];
                        }
                        dst[2] = mix(7, 1, w[4], w[5]);
                        dst[3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride + 3] = mix(5, 3, w[4], w[5]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                            dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                            dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride * 2 + 1] = w[4];
                            dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[7]);
                        }
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 2 + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 3 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[5]);
                        break;
                    }
                case 205:
                case 201:
                    {
                        dst[0] = mix(5, 3, w[4], w[1]);
                        dst[1] = mix(5, 3, w[4], w[1]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[5]);
                        dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 2] = mix(6, 1, 1, w[4], w[5], w[1]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[1]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                            dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                            dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride * 2 + 1] = w[4];
                            dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[7]);
                        }
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 2 + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 3 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[5]);
                        break;
                    }
                case 174:
                case 46:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = mix(5, 3, w[4], w[0]);
                            dst[1] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride] = mix(3, 1, w[4], w[0]);
                            dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                            dst[1] = mix(3, 1, w[4], w[1]);
                            dst[dst_stride] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride + 1] = w[4];
                        }
                        dst[2] = mix(7, 1, w[4], w[5]);
                        dst[3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 2] = mix(6, 1, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 1] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[5]);
                        dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        break;
                    }
                case 179:
                case 147:
                    {
                        dst[0] = mix(5, 3, w[4], w[3]);
                        dst[1] = mix(7, 1, w[4], w[3]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = mix(3, 1, w[4], w[2]);
                            dst[3] = mix(5, 3, w[4], w[2]);
                            dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        }
                        else
                        {
                            dst[2] = mix(3, 1, w[4], w[1]);
                            dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + 3] = mix(3, 1, w[4], w[5]);
                        }
                        dst[dst_stride] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 1] = mix(6, 1, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 3] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 2] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[7]);
                        break;
                    }
                case 117:
                case 116:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[3]);
                        dst[2] = mix(5, 3, w[4], w[1]);
                        dst[3] = mix(5, 3, w[4], w[1]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 3] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride * 2] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[3]);
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                            dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        }
                        else
                        {
                            dst[dst_stride * 2 + 2] = w[4];
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[5]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[7]);
                            dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        }
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 3 + 1] = mix(7, 1, w[4], w[3]);
                        break;
                    }
                case 189:
                    {
                        dst[0] = mix(5, 3, w[4], w[1]);
                        dst[1] = mix(5, 3, w[4], w[1]);
                        dst[2] = mix(5, 3, w[4], w[1]);
                        dst[3] = mix(5, 3, w[4], w[1]);
                        dst[dst_stride] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 3] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride * 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 3] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 1] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 2] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[7]);
                        break;
                    }
                case 231:
                    {
                        dst[0] = mix(5, 3, w[4], w[3]);
                        dst[1] = mix(7, 1, w[4], w[3]);
                        dst[2] = mix(7, 1, w[4], w[5]);
                        dst[3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 2] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 2 + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 3 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 3 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[5]);
                        break;
                    }
                case 126:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                            dst[3] = w[4];
                            dst[dst_stride + 3] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(1, 1, w[1], w[4]);
                            dst[3] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 3] = mix(1, 1, w[5], w[4]);
                        }
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = w[4];
                            dst[dst_stride * 3] = w[4];
                            dst[dst_stride * 3 + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(1, 1, w[3], w[4]);
                            dst[dst_stride * 3] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(1, 1, w[7], w[4]);
                        }
                        dst[dst_stride * 2 + 1] = w[4];
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 219:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[1], w[3]);
                            dst[1] = mix(1, 1, w[1], w[4]);
                            dst[dst_stride] = mix(1, 1, w[3], w[4]);
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 3] = w[4];
                            dst[dst_stride * 3 + 2] = w[4];
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2 + 3] = mix(1, 1, w[5], w[4]);
                            dst[dst_stride * 3 + 2] = mix(1, 1, w[7], w[4]);
                            dst[dst_stride * 3 + 3] = mix(1, 1, w[7], w[5]);
                        }
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        break;
                    }
                case 125:
                    {
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[0] = mix(5, 3, w[4], w[1]);
                            dst[dst_stride] = mix(7, 1, w[4], w[1]);
                            dst[dst_stride * 2] = w[4];
                            dst[dst_stride * 2 + 1] = w[4];
                            dst[dst_stride * 3] = w[4];
                            dst[dst_stride * 3 + 1] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(3, 1, w[4], w[3]);
                            dst[dst_stride] = mix(3, 1, w[3], w[4]);
                            dst[dst_stride * 2] = mix(5, 3, w[3], w[7]);
                            dst[dst_stride * 2 + 1] = mix(6, 1, 1, w[4], w[3], w[7]);
                            dst[dst_stride * 3] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(2, 1, 1, w[7], w[4], w[3]);
                        }
                        dst[1] = mix(5, 3, w[4], w[1]);
                        dst[2] = mix(5, 3, w[4], w[1]);
                        dst[3] = mix(5, 3, w[4], w[1]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 3] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 221:
                    {
                        dst[0] = mix(5, 3, w[4], w[1]);
                        dst[1] = mix(5, 3, w[4], w[1]);
                        dst[2] = mix(5, 3, w[4], w[1]);
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[3] = mix(5, 3, w[4], w[1]);
                            dst[dst_stride + 3] = mix(7, 1, w[4], w[1]);
                            dst[dst_stride * 2 + 2] = w[4];
                            dst[dst_stride * 2 + 3] = w[4];
                            dst[dst_stride * 3 + 2] = w[4];
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[3] = mix(3, 1, w[4], w[5]);
                            dst[dst_stride + 3] = mix(3, 1, w[5], w[4]);
                            dst[dst_stride * 2 + 2] = mix(6, 1, 1, w[4], w[5], w[7]);
                            dst[dst_stride * 2 + 3] = mix(5, 3, w[5], w[7]);
                            dst[dst_stride * 3 + 2] = mix(2, 1, 1, w[7], w[4], w[5]);
                            dst[dst_stride * 3 + 3] = mix(1, 1, w[7], w[5]);
                        }
                        dst[dst_stride] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        break;
                    }
                case 207:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[2] = mix(7, 1, w[4], w[5]);
                            dst[3] = mix(5, 3, w[4], w[5]);
                            dst[dst_stride] = w[4];
                            dst[dst_stride + 1] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[1], w[3]);
                            dst[1] = mix(5, 3, w[1], w[3]);
                            dst[2] = mix(3, 1, w[1], w[4]);
                            dst[3] = mix(3, 1, w[4], w[1]);
                            dst[dst_stride] = mix(2, 1, 1, w[3], w[4], w[1]);
                            dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[3], w[1]);
                        }
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 2 + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 3 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[5]);
                        break;
                    }
                case 238:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        dst[2] = mix(7, 1, w[4], w[5]);
                        dst[3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride + 3] = mix(5, 3, w[4], w[5]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = w[4];
                            dst[dst_stride * 2 + 1] = w[4];
                            dst[dst_stride * 3] = w[4];
                            dst[dst_stride * 3 + 1] = w[4];
                            dst[dst_stride * 3 + 2] = mix(7, 1, w[4], w[5]);
                            dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[5]);
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(2, 1, 1, w[3], w[4], w[7]);
                            dst[dst_stride * 2 + 1] = mix(6, 1, 1, w[4], w[3], w[7]);
                            dst[dst_stride * 3] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(5, 3, w[7], w[3]);
                            dst[dst_stride * 3 + 2] = mix(3, 1, w[7], w[4]);
                            dst[dst_stride * 3 + 3] = mix(3, 1, w[4], w[7]);
                        }
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 2 + 3] = mix(5, 3, w[4], w[5]);
                        break;
                    }
                case 190:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                            dst[3] = w[4];
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + 3] = w[4];
                            dst[dst_stride * 2 + 3] = mix(7, 1, w[4], w[7]);
                            dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[7]);
                        }
                        else
                        {
                            dst[2] = mix(2, 1, 1, w[1], w[4], w[5]);
                            dst[3] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 2] = mix(6, 1, 1, w[4], w[5], w[1]);
                            dst[dst_stride + 3] = mix(5, 3, w[5], w[1]);
                            dst[dst_stride * 2 + 3] = mix(3, 1, w[5], w[4]);
                            dst[dst_stride * 3 + 3] = mix(3, 1, w[4], w[5]);
                        }
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride * 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 1] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 2] = mix(5, 3, w[4], w[7]);
                        break;
                    }
                case 187:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                            dst[dst_stride + 1] = w[4];
                            dst[dst_stride * 2] = mix(7, 1, w[4], w[7]);
                            dst[dst_stride * 3] = mix(5, 3, w[4], w[7]);
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[1], w[3]);
                            dst[1] = mix(2, 1, 1, w[1], w[4], w[3]);
                            dst[dst_stride] = mix(5, 3, w[3], w[1]);
                            dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[3], w[1]);
                            dst[dst_stride * 2] = mix(3, 1, w[3], w[4]);
                            dst[dst_stride * 3] = mix(3, 1, w[4], w[3]);
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 3] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 3 + 1] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 2] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[7]);
                        break;
                    }
                case 243:
                    {
                        dst[0] = mix(5, 3, w[4], w[3]);
                        dst[1] = mix(7, 1, w[4], w[3]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride * 2] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[3]);
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 2] = w[4];
                            dst[dst_stride * 2 + 3] = w[4];
                            dst[dst_stride * 3] = mix(5, 3, w[4], w[3]);
                            dst[dst_stride * 3 + 1] = mix(7, 1, w[4], w[3]);
                            dst[dst_stride * 3 + 2] = w[4];
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2 + 2] = mix(6, 1, 1, w[4], w[5], w[7]);
                            dst[dst_stride * 2 + 3] = mix(2, 1, 1, w[5], w[4], w[7]);
                            dst[dst_stride * 3] = mix(3, 1, w[4], w[7]);
                            dst[dst_stride * 3 + 1] = mix(3, 1, w[7], w[4]);
                            dst[dst_stride * 3 + 2] = mix(5, 3, w[7], w[5]);
                            dst[dst_stride * 3 + 3] = mix(1, 1, w[7], w[5]);
                        }
                        break;
                    }
                case 119:
                    {
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[0] = mix(5, 3, w[4], w[3]);
                            dst[1] = mix(7, 1, w[4], w[3]);
                            dst[2] = w[4];
                            dst[3] = w[4];
                            dst[dst_stride + 2] = w[4];
                            dst[dst_stride + 3] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(3, 1, w[4], w[1]);
                            dst[1] = mix(3, 1, w[1], w[4]);
                            dst[2] = mix(5, 3, w[1], w[5]);
                            dst[3] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 2] = mix(6, 1, 1, w[4], w[5], w[1]);
                            dst[dst_stride + 3] = mix(2, 1, 1, w[5], w[4], w[1]);
                        }
                        dst[dst_stride] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 2] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 3 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 237:
                case 233:
                    {
                        dst[0] = mix(5, 3, w[4], w[1]);
                        dst[1] = mix(5, 3, w[4], w[1]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[5]);
                        dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                        dst[dst_stride] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 2] = mix(6, 1, 1, w[4], w[5], w[1]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[1]);
                        dst[dst_stride * 2] = w[4];
                        dst[dst_stride * 2 + 1] = w[4];
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 2 + 3] = mix(5, 3, w[4], w[5]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        dst[dst_stride * 3 + 1] = w[4];
                        dst[dst_stride * 3 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[5]);
                        break;
                    }
                case 175:
                case 47:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                        }
                        dst[1] = w[4];
                        dst[2] = mix(7, 1, w[4], w[5]);
                        dst[3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 2] = mix(6, 1, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[7]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 1] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[5]);
                        dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        break;
                    }
                case 183:
                case 151:
                    {
                        dst[0] = mix(5, 3, w[4], w[3]);
                        dst[1] = mix(7, 1, w[4], w[3]);
                        dst[2] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[3] = w[4];
                        }
                        else
                        {
                            dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                        }
                        dst[dst_stride] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + 3] = w[4];
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 1] = mix(6, 1, 1, w[4], w[3], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 3] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[3]);
                        dst[dst_stride * 3 + 2] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[7]);
                        break;
                    }
                case 245:
                case 244:
                    {
                        dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[3]);
                        dst[2] = mix(5, 3, w[4], w[1]);
                        dst[3] = mix(5, 3, w[4], w[1]);
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 1] = mix(6, 1, 1, w[4], w[3], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 3] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride * 2] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 2 + 2] = w[4];
                        dst[dst_stride * 2 + 3] = w[4];
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 3 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 3 + 2] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        }
                        break;
                    }
                case 250:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = w[4];
                            dst[dst_stride * 3] = w[4];
                            dst[dst_stride * 3 + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(1, 1, w[3], w[4]);
                            dst[dst_stride * 3] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(1, 1, w[7], w[4]);
                        }
                        dst[dst_stride * 2 + 1] = w[4];
                        dst[dst_stride * 2 + 2] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 3] = w[4];
                            dst[dst_stride * 3 + 2] = w[4];
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2 + 3] = mix(1, 1, w[5], w[4]);
                            dst[dst_stride * 3 + 2] = mix(1, 1, w[7], w[4]);
                            dst[dst_stride * 3 + 3] = mix(1, 1, w[7], w[5]);
                        }
                        break;
                    }
                case 123:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[1], w[3]);
                            dst[1] = mix(1, 1, w[1], w[4]);
                            dst[dst_stride] = mix(1, 1, w[3], w[4]);
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = w[4];
                            dst[dst_stride * 3] = w[4];
                            dst[dst_stride * 3 + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(1, 1, w[3], w[4]);
                            dst[dst_stride * 3] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(1, 1, w[7], w[4]);
                        }
                        dst[dst_stride * 2 + 1] = w[4];
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 95:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[1], w[3]);
                            dst[1] = mix(1, 1, w[1], w[4]);
                            dst[dst_stride] = mix(1, 1, w[3], w[4]);
                        }
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                            dst[3] = w[4];
                            dst[dst_stride + 3] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(1, 1, w[1], w[4]);
                            dst[3] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 3] = mix(1, 1, w[5], w[4]);
                        }
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 222:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                            dst[3] = w[4];
                            dst[dst_stride + 3] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(1, 1, w[1], w[4]);
                            dst[3] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 3] = mix(1, 1, w[5], w[4]);
                        }
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 3] = w[4];
                            dst[dst_stride * 3 + 2] = w[4];
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2 + 3] = mix(1, 1, w[5], w[4]);
                            dst[dst_stride * 3 + 2] = mix(1, 1, w[7], w[4]);
                            dst[dst_stride * 3 + 3] = mix(1, 1, w[7], w[5]);
                        }
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        break;
                    }
                case 252:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(5, 2, 1, w[4], w[1], w[0]);
                        dst[2] = mix(5, 3, w[4], w[1]);
                        dst[3] = mix(5, 3, w[4], w[1]);
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 3] = mix(7, 1, w[4], w[1]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = w[4];
                            dst[dst_stride * 3] = w[4];
                            dst[dst_stride * 3 + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(1, 1, w[3], w[4]);
                            dst[dst_stride * 3] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(1, 1, w[7], w[4]);
                        }
                        dst[dst_stride * 2 + 1] = w[4];
                        dst[dst_stride * 2 + 2] = w[4];
                        dst[dst_stride * 2 + 3] = w[4];
                        dst[dst_stride * 3 + 2] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        }
                        break;
                    }
                case 249:
                    {
                        dst[0] = mix(5, 3, w[4], w[1]);
                        dst[1] = mix(5, 3, w[4], w[1]);
                        dst[2] = mix(5, 2, 1, w[4], w[1], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride * 2] = w[4];
                        dst[dst_stride * 2 + 1] = w[4];
                        dst[dst_stride * 2 + 2] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 3] = w[4];
                            dst[dst_stride * 3 + 2] = w[4];
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2 + 3] = mix(1, 1, w[5], w[4]);
                            dst[dst_stride * 3 + 2] = mix(1, 1, w[7], w[4]);
                            dst[dst_stride * 3 + 3] = mix(1, 1, w[7], w[5]);
                        }
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        dst[dst_stride * 3 + 1] = w[4];
                        break;
                    }
                case 235:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[1], w[3]);
                            dst[1] = mix(1, 1, w[1], w[4]);
                            dst[dst_stride] = mix(1, 1, w[3], w[4]);
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(5, 2, 1, w[4], w[5], w[2]);
                        dst[dst_stride * 2] = w[4];
                        dst[dst_stride * 2 + 1] = w[4];
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 2 + 3] = mix(5, 3, w[4], w[5]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        dst[dst_stride * 3 + 1] = w[4];
                        dst[dst_stride * 3 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[5]);
                        break;
                    }
                case 111:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                        }
                        dst[1] = w[4];
                        dst[2] = mix(7, 1, w[4], w[5]);
                        dst[3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride + 3] = mix(5, 3, w[4], w[5]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = w[4];
                            dst[dst_stride * 3] = w[4];
                            dst[dst_stride * 3 + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(1, 1, w[3], w[4]);
                            dst[dst_stride * 3] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(1, 1, w[7], w[4]);
                        }
                        dst[dst_stride * 2 + 1] = w[4];
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(5, 2, 1, w[4], w[5], w[8]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 63:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                        }
                        dst[1] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                            dst[3] = w[4];
                            dst[dst_stride + 3] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(1, 1, w[1], w[4]);
                            dst[3] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 3] = mix(1, 1, w[5], w[4]);
                        }
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride * 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 1] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 2] = mix(5, 2, 1, w[4], w[7], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 159:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[1], w[3]);
                            dst[1] = mix(1, 1, w[1], w[4]);
                            dst[dst_stride] = mix(1, 1, w[3], w[4]);
                        }
                        dst[2] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[3] = w[4];
                        }
                        else
                        {
                            dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                        }
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + 3] = w[4];
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 3] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(5, 2, 1, w[4], w[7], w[6]);
                        dst[dst_stride * 3 + 2] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[7]);
                        break;
                    }
                case 215:
                    {
                        dst[0] = mix(5, 3, w[4], w[3]);
                        dst[1] = mix(7, 1, w[4], w[3]);
                        dst[2] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[3] = w[4];
                        }
                        else
                        {
                            dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                        }
                        dst[dst_stride] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + 3] = w[4];
                        dst[dst_stride * 2] = mix(5, 2, 1, w[4], w[3], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 3] = w[4];
                            dst[dst_stride * 3 + 2] = w[4];
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2 + 3] = mix(1, 1, w[5], w[4]);
                            dst[dst_stride * 3 + 2] = mix(1, 1, w[7], w[4]);
                            dst[dst_stride * 3 + 3] = mix(1, 1, w[7], w[5]);
                        }
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        break;
                    }
                case 246:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                            dst[3] = w[4];
                            dst[dst_stride + 3] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(1, 1, w[1], w[4]);
                            dst[3] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 3] = mix(1, 1, w[5], w[4]);
                        }
                        dst[dst_stride] = mix(5, 2, 1, w[4], w[3], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride * 2] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 2 + 2] = w[4];
                        dst[dst_stride * 2 + 3] = w[4];
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 3 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 3 + 2] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        }
                        break;
                    }
                case 254:
                    {
                        dst[0] = mix(5, 3, w[4], w[0]);
                        dst[1] = mix(3, 1, w[4], w[0]);
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                            dst[3] = w[4];
                            dst[dst_stride + 3] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(1, 1, w[1], w[4]);
                            dst[3] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 3] = mix(1, 1, w[5], w[4]);
                        }
                        dst[dst_stride] = mix(3, 1, w[4], w[0]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[0]);
                        dst[dst_stride + 2] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = w[4];
                            dst[dst_stride * 3] = w[4];
                            dst[dst_stride * 3 + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(1, 1, w[3], w[4]);
                            dst[dst_stride * 3] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(1, 1, w[7], w[4]);
                        }
                        dst[dst_stride * 2 + 1] = w[4];
                        dst[dst_stride * 2 + 2] = w[4];
                        dst[dst_stride * 2 + 3] = w[4];
                        dst[dst_stride * 3 + 2] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        }
                        break;
                    }
                case 253:
                    {
                        dst[0] = mix(5, 3, w[4], w[1]);
                        dst[1] = mix(5, 3, w[4], w[1]);
                        dst[2] = mix(5, 3, w[4], w[1]);
                        dst[3] = mix(5, 3, w[4], w[1]);
                        dst[dst_stride] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride + 3] = mix(7, 1, w[4], w[1]);
                        dst[dst_stride * 2] = w[4];
                        dst[dst_stride * 2 + 1] = w[4];
                        dst[dst_stride * 2 + 2] = w[4];
                        dst[dst_stride * 2 + 3] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        dst[dst_stride * 3 + 1] = w[4];
                        dst[dst_stride * 3 + 2] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        }
                        break;
                    }
                case 251:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[1], w[3]);
                            dst[1] = mix(1, 1, w[1], w[4]);
                            dst[dst_stride] = mix(1, 1, w[3], w[4]);
                        }
                        dst[2] = mix(3, 1, w[4], w[2]);
                        dst[3] = mix(5, 3, w[4], w[2]);
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[2]);
                        dst[dst_stride + 3] = mix(3, 1, w[4], w[2]);
                        dst[dst_stride * 2] = w[4];
                        dst[dst_stride * 2 + 1] = w[4];
                        dst[dst_stride * 2 + 2] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 3] = w[4];
                            dst[dst_stride * 3 + 2] = w[4];
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2 + 3] = mix(1, 1, w[5], w[4]);
                            dst[dst_stride * 3 + 2] = mix(1, 1, w[7], w[4]);
                            dst[dst_stride * 3 + 3] = mix(1, 1, w[7], w[5]);
                        }
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        dst[dst_stride * 3 + 1] = w[4];
                        break;
                    }
                case 239:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                        }
                        dst[1] = w[4];
                        dst[2] = mix(7, 1, w[4], w[5]);
                        dst[3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride + 3] = mix(5, 3, w[4], w[5]);
                        dst[dst_stride * 2] = w[4];
                        dst[dst_stride * 2 + 1] = w[4];
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 2 + 3] = mix(5, 3, w[4], w[5]);
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        dst[dst_stride * 3 + 1] = w[4];
                        dst[dst_stride * 3 + 2] = mix(7, 1, w[4], w[5]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[5]);
                        break;
                    }
                case 127:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                        }
                        dst[1] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[2] = w[4];
                            dst[3] = w[4];
                            dst[dst_stride + 3] = w[4];
                        }
                        else
                        {
                            dst[2] = mix(1, 1, w[1], w[4]);
                            dst[3] = mix(1, 1, w[1], w[5]);
                            dst[dst_stride + 3] = mix(1, 1, w[5], w[4]);
                        }
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2] = w[4];
                            dst[dst_stride * 3] = w[4];
                            dst[dst_stride * 3 + 1] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2] = mix(1, 1, w[3], w[4]);
                            dst[dst_stride * 3] = mix(1, 1, w[7], w[3]);
                            dst[dst_stride * 3 + 1] = mix(1, 1, w[7], w[4]);
                        }
                        dst[dst_stride * 2 + 1] = w[4];
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[8]);
                        dst[dst_stride * 2 + 3] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 2] = mix(3, 1, w[4], w[8]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[8]);
                        break;
                    }
                case 191:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                        }
                        dst[1] = w[4];
                        dst[2] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[3] = w[4];
                        }
                        else
                        {
                            dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                        }
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + 3] = w[4];
                        dst[dst_stride * 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 2] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 2 + 3] = mix(7, 1, w[4], w[7]);
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 1] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 2] = mix(5, 3, w[4], w[7]);
                        dst[dst_stride * 3 + 3] = mix(5, 3, w[4], w[7]);
                        break;
                    }
                case 223:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                            dst[1] = w[4];
                            dst[dst_stride] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(1, 1, w[1], w[3]);
                            dst[1] = mix(1, 1, w[1], w[4]);
                            dst[dst_stride] = mix(1, 1, w[3], w[4]);
                        }
                        dst[2] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[3] = w[4];
                        }
                        else
                        {
                            dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                        }
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + 3] = w[4];
                        dst[dst_stride * 2] = mix(3, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[6]);
                        dst[dst_stride * 2 + 2] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 2 + 3] = w[4];
                            dst[dst_stride * 3 + 2] = w[4];
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 2 + 3] = mix(1, 1, w[5], w[4]);
                            dst[dst_stride * 3 + 2] = mix(1, 1, w[7], w[4]);
                            dst[dst_stride * 3 + 3] = mix(1, 1, w[7], w[5]);
                        }
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[6]);
                        dst[dst_stride * 3 + 1] = mix(3, 1, w[4], w[6]);
                        break;
                    }
                case 247:
                    {
                        dst[0] = mix(5, 3, w[4], w[3]);
                        dst[1] = mix(7, 1, w[4], w[3]);
                        dst[2] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[3] = w[4];
                        }
                        else
                        {
                            dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                        }
                        dst[dst_stride] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + 3] = w[4];
                        dst[dst_stride * 2] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 2 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 2 + 2] = w[4];
                        dst[dst_stride * 2 + 3] = w[4];
                        dst[dst_stride * 3] = mix(5, 3, w[4], w[3]);
                        dst[dst_stride * 3 + 1] = mix(7, 1, w[4], w[3]);
                        dst[dst_stride * 3 + 2] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        }
                        break;
                    }
                case 255:
                    {
                        if (diff(w[3], w[1], trY, trU, trV, trA))
                        {
                            dst[0] = w[4];
                        }
                        else
                        {
                            dst[0] = mix(2, 1, 1, w[4], w[1], w[3]);
                        }
                        dst[1] = w[4];
                        dst[2] = w[4];
                        if (diff(w[1], w[5], trY, trU, trV, trA))
                        {
                            dst[3] = w[4];
                        }
                        else
                        {
                            dst[3] = mix(2, 1, 1, w[4], w[1], w[5]);
                        }
                        dst[dst_stride] = w[4];
                        dst[dst_stride + 1] = w[4];
                        dst[dst_stride + 2] = w[4];
                        dst[dst_stride + 3] = w[4];
                        dst[dst_stride * 2] = w[4];
                        dst[dst_stride * 2 + 1] = w[4];
                        dst[dst_stride * 2 + 2] = w[4];
                        dst[dst_stride * 2 + 3] = w[4];
                        if (diff(w[7], w[3], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 3] = mix(2, 1, 1, w[4], w[7], w[3]);
                        }
                        dst[dst_stride * 3 + 1] = w[4];
                        dst[dst_stride * 3 + 2] = w[4];
                        if (diff(w[5], w[7], trY, trU, trV, trA))
                        {
                            dst[dst_stride * 3 + 3] = w[4];
                        }
                        else
                        {
                            dst[dst_stride * 3 + 3] = mix(2, 1, 1, w[4], w[7], w[5]);
                        }
                        break;
                    }
            }
            src++;
            dst += 4;
        }
        src += src_stride - src_size.x;
        dst += dst_stride - src_size.x * 4;
        dst += dst_stride * 3;
    }
}

static void hqxxProcess(const uint32_t* src, uint32_t* dst, sp::Vector2i src_size, int src_stride, HQ2xConfig config)
{
    switch(config.scale)
    {
    case 2: hq2xProcess(src, dst, src_size, src_stride, config); break;
    case 3: hq3xProcess(src, dst, src_size, src_stride, config); break;
    case 4: hq4xProcess(src, dst, src_size, src_stride, config); break;
    }
}

void hq2x(sf::Image& image, HQ2xConfig config)
{
    if (config.scale < 2)
        return;
    if (config.scale > 4)
        config.scale = 4;
    sp::Vector2i size(image.getSize().x, image.getSize().y);
    uint32_t* target_buffer = new uint32_t[size.x * size.y * config.scale * config.scale];
    
    const uint32_t* src = (const uint32_t*)image.getPixelsPtr();
    uint32_t* dst = target_buffer;

    hqxxProcess(src, dst, size, size.x, config);

    image.create(size.x * config.scale, size.y * config.scale, (uint8_t*)target_buffer);
    delete[] target_buffer;
}

void hq2xTiles(sf::Image& image, sp::Vector2i tile_size, HQ2xConfig config)
{
    if (config.scale < 2)
        return;
    if (config.scale > 4)
        config.scale = 4;
    sp::Vector2i size(image.getSize().x, image.getSize().y);
    uint32_t* target_buffer = new uint32_t[size.x * size.y * config.scale * config.scale];
    
    const uint32_t* src = (const uint32_t*)image.getPixelsPtr();
    uint32_t* dst = target_buffer;

    //hqxxProcess(src, dst, size, size.x, config);
    for(int y=0; y<size.y; y+=tile_size.y)
    {
        for(int x=0; x<size.x; x+=tile_size.x)
        {
            hqxxProcess(src + x + y * size.x, dst + x * config.scale + y * size.x * config.scale * config.scale, sp::Vector2i(std::min(tile_size.x, size.x - x), std::min(tile_size.y, size.y - y)), size.x, config);
        }
    }

    image.create(size.x * config.scale, size.y * config.scale, (uint8_t*)target_buffer);
    delete[] target_buffer;
}

};//namespace image
};//namespace sp
