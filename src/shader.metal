#include <metal_stdlib>

#include "constants.hpp"
#include "sprite.hpp"

using namespace metal;

[[kernel]] void k_clear_screen(texture2d<float, access::read_write> out_texture
        [[texture(0)]],
        uint2 gid [[thread_position_in_grid]])
{
    if (gid.x >= out_texture.get_width() || gid.y >= out_texture.get_height()) {
        return;
    }

    float4 bg_color = float4(0.06, 0.22, 0.06, 1.0);

    out_texture.write(bg_color, gid);
}

[[kernel]] void k_draw_sprites(constant sc::sprite* sprites [[buffer(0)]],
        constant float* x_coords [[buffer(1)]],
        constant float* y_coords [[buffer(2)]],
        constant uint64_t* sprite_ids [[buffer(3)]],
        texture2d<float, access::read_write> out_texture [[texture(0)]],
        uint2 gid [[threadgroup_position_in_grid]],
        uint2 tid [[thread_position_in_threadgroup]])
{
    const uint entity_idx{gid.x};
    const auto entity_coord{float2(x_coords[entity_idx], y_coords[entity_idx])};
    const auto sscreen_coord{int2(entity_coord) + int2(tid)};
    if (sscreen_coord.x >= sc::display::SCREEN_WIDTH ||
            sscreen_coord.y >= sc::display::SCREEN_HEIGHT)
        return;

    const auto screen_coord{uint2(sscreen_coord)};
    constant sc::sprite& sprite{sprites[sprite_ids[entity_idx]]};
    const auto pixel{sprite.pixels[tid.y * sc::SPRITE_WIDTH + tid.x]};

    if (pixel.alpha == 0x00)
        return;

    const ushort packed_color{sprite.palette[pixel.index]};
    float r{1.0f}, g{1.0f}, b{1.0f};
    switch (sprite.encoding) {
    case sc::color_encoding::DEFAULT:
        r = ((packed_color >> 11) & 0x1F) / 31.0f;
        g = ((packed_color >> 5) & 0x3F) / 63.0f;
        b = (packed_color & 0x1F) / 31.0f;
        break;
    case sc::color_encoding::WARM:
        r = ((packed_color >> 10) & 0x3F) / 63.0f;
        g = ((packed_color >> 5) & 0x1F) / 31.0f;
        b = (packed_color & 0x1F) / 31.0f;
        break;
    case sc::color_encoding::COOL:
        r = ((packed_color >> 11) & 0x1F) / 31.0f;
        g = ((packed_color >> 5) & 0x1F) / 31.0f;
        b = (packed_color & 0x3F) / 63.0f;
        break;
    default:
        break;
    }
    const float a{pixel.alpha / 3.0f};
    const auto sprite_color{float4(r, g, b, a)};
    float4 out_color;
    if (a < 1) {
        const float4 background{out_texture.read(screen_coord)};
        out_color = (sprite_color * a) + (background * (1.0f - a));
        out_color.a = 1.0f;
    }
    else
        out_color = sprite_color;

    out_texture.write(out_color, screen_coord);
}
