#include <metal_stdlib>

#define HEIGHT 32
#define WIDTH 32
#define MAX_PALETTE_SIZE 16

enum class color_encoding : uchar {
    DEFAULT = 1, // R5G6B5
    WARM, // R6G5B5
    COOL, // R5G5B6
};

using color = ushort;

struct pixel {
    uchar index : 4;
    uchar alpha : 2;
    uchar glow : 1;
    uchar reserved : 1;
};

struct alignas(16) sprite {
    uchar min_x, min_y;
    uchar max_x, max_y;
    uchar anchor_x, anchor_y;
    color_encoding encoding;
    uchar reserved;
    color palette[MAX_PALETTE_SIZE];
    pixel pixels[HEIGHT * WIDTH];
    ulong padding;
};

[[kernel]] void render_sprite(constant sprite& sprite [[buffer(0)]],
        metal::texture2d<float, metal::access::write> out_texture
        [[texture(0)]],
        uint2 gid [[thread_position_in_grid]])
{
    const auto pixel{sprite.pixels[gid.y * WIDTH + gid.x]};

    const ushort color{sprite.palette[pixel.index]};
    float r, g, b;
    switch (sprite.encoding) {
    case color_encoding::DEFAULT:
        r = ((color >> 11) & 0x1F) / 31.0;
        g = ((color >> 5) & 0x3F) / 63.0;
        b = (color & 0x1F) / 31.0;
        break;
    case color_encoding::WARM:
        r = ((color >> 10) & 0x3F) / 63.0;
        g = ((color >> 5) & 0x1F) / 31.0;
        b = (color & 0x1F) / 31.0;
        break;
    case color_encoding::COOL:
        r = ((color >> 11) & 0x1F) / 31.0;
        g = ((color >> 5) & 0x1F) / 31.0;
        b = (color & 0x3F) / 63.0;
        break;
    default:
        r = 0;
        g = 0;
        b = 0;
        break;
    }
    const float a{pixel.alpha / 3.0};

    out_texture.write(float4(r, g, b, a), gid);
}
