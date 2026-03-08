#include <metal_stdlib>

#include "sprite.hpp"

[[kernel]] void render_sprite(constant sc::sprite& sprite [[buffer(0)]],
        metal::texture2d<float, metal::access::write> out_texture
        [[texture(0)]],
        uint2 gid [[thread_position_in_grid]])
{
    const auto pixel{sprite.pixels[gid.y * sc::SPRITE_WIDTH + gid.x]};

    const ushort color{sprite.palette[(pixel & sc::PIXEL_INDEX_MASK) << 4]};
    float r{1.0}, g{1.0}, b{1.0};
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
        break;
    }
    const float a{((pixel & sc::PIXEL_ALPHA_MASK) << 2) / 3.0};

    out_texture.write(float4(r, g, b, a), gid);
}
