#include <metal_stdlib>

#include "sprite.hpp"

[[kernel]] void render_sprite(constant sc::sprite& sprite [[buffer(0)]],
        metal::texture2d<float, metal::access::write> out_texture
        [[texture(0)]],
        uint2 gid [[thread_position_in_grid]])
{
    const auto pixel{sprite.pixels[gid.y * SC_SPRITE_WIDTH + gid.x]};

    const ushort color{sprite.palette[pixel.index]};
    float r, g, b;
    switch (sprite.encoding) {
    case sc::color_encoding::DEFAULT:
        r = ((color >> 11) & 0x1F) / 31.0;
        g = ((color >> 5) & 0x3F) / 63.0;
        b = (color & 0x1F) / 31.0;
        break;
    case sc::color_encoding::WARM:
        r = ((color >> 10) & 0x3F) / 63.0;
        g = ((color >> 5) & 0x1F) / 31.0;
        b = (color & 0x1F) / 31.0;
        break;
    case sc::color_encoding::COOL:
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
