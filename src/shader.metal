#include <metal_stdlib>
using namespace metal;

#define WIDTH 32
#define HEIGHT 32
#define MAX_PALETTE_SIZE 16

enum ColorMode : uchar {
    DEFAULT = 1, // R5G6B5
    WARM, // R6G5B5
    COOL, // R5G5B6
    FOREST, // R3G10B3
    SEA, // R2G2B12
};

using Color = ushort;

struct Pixel {
    uchar index : 4;
    uchar alpha : 2;
    uchar reserved : 2;
};

struct alignas(32) Sprite {
    uchar magic[8];
    ColorMode mode;
    uchar anchor_x;
    uchar anchor_y;
    uchar reserved[21];
    Color palette[MAX_PALETTE_SIZE];
    Pixel pixels[WIDTH * HEIGHT];
};

kernel void sprite_render(constant Sprite& sprite [[buffer(0)]],
        texture2d<float, access::write> outTexture [[texture(0)]],
        uint2 gid [[thread_position_in_grid]])
{
    const auto pixel{sprite.pixels[gid.y * HEIGHT + gid.x]};

    const ushort color{sprite.palette[pixel.index]};
    float r, g, b;
    switch (sprite.mode) {
    case DEFAULT:
        r = ((color >> 11) & 0x1F) / 31.0;
        g = ((color >> 5) & 0x3F) / 63.0;
        b = (color & 0x1F) / 31.0;
        break;
    case WARM:
        r = ((color >> 10) & 0x3F) / 63.0;
        g = ((color >> 5) & 0x1F) / 31.0;
        b = (color & 0x1F) / 31.0;
        break;
    case COOL:
        r = ((color >> 11) & 0x1F) / 31.0;
        g = ((color >> 5) & 0x1F) / 31.0;
        b = (color & 0x3F) / 63.0;
        break;
    case FOREST:
        r = ((color >> 13) & 0x07) / 7.0;
        g = ((color >> 3) & 0x3FF) / 1023.0;
        b = (color & 0x07) / 7.0;
        break;
    case SEA:
        r = ((color >> 14) & 0x03) / 3.0;
        g = ((color >> 12) & 0x03) / 3.0;
        b = (color & 0xFFF) / 4095.0;
        break;
    default:
        r = 0;
        g = 0;
        b = 0;
        break;
    }
    const float a{pixel.alpha / 3.0};

    outTexture.write(float4(r, g, b, a), gid);
}
