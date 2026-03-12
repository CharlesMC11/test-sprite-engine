#include <metal_stdlib>

#include "core.hh"
#include "sprite.hh"

using namespace metal;

[[kernel]] void k_clear_screen(texture2d<float, access::read_write> out_texture
        [[texture(0)]],
        uint2 gid [[thread_position_in_grid]])
{
    if (gid.x >= out_texture.get_width() || gid.y >= out_texture.get_height())
        return;

    constexpr auto bg_color{float4(sc::display::kDefaultR,
            sc::display::kDefaultG, sc::display::kDefaultB, 1.0f)};

    out_texture.write(bg_color, gid);
}

inline float4 unpack_color(
        constant sc::sprites::sprite& sprite, sc::sprites::packed_pixel p)
{
    const ushort packed_color{sprite.palette[p.index]};
    float r{1.0f}, g{1.0f}, b{1.0f};
    switch (sprite.encoding) {
    case sc::sprites::color_encoding::DEFAULT:
        r = static_cast<float>((packed_color >> 11) & 0x1F) / 31.0f;
        g = static_cast<float>((packed_color >> 5) & 0x3F) / 63.0f;
        b = static_cast<float>(packed_color & 0x1F) / 31.0f;
        break;
    case sc::sprites::color_encoding::WARM:
        r = static_cast<float>((packed_color >> 10) & 0x3F) / 63.0f;
        g = static_cast<float>((packed_color >> 5) & 0x1F) / 31.0f;
        b = static_cast<float>(packed_color & 0x1F) / 31.0f;
        break;
    case sc::sprites::color_encoding::COOL:
        r = static_cast<float>((packed_color >> 11) & 0x1F) / 31.0f;
        g = static_cast<float>((packed_color >> 5) & 0x1F) / 31.0f;
        b = static_cast<float>(packed_color & 0x3F) / 63.0f;
        break;
    default:
        break;
    }

    return float4(r, g, b, 1.0f);
}

[[kernel]] void k_draw_sprites(constant sc::sprites::sprite* sprites
        [[buffer(0)]],
        constant float* x_coords [[buffer(1)]],
        constant float* y_coords [[buffer(2)]],
        constant float* z_coords [[buffer(3)]],
        constant sc::core::atlas_index_t* sprite_ids [[buffer(4)]],
        constant sc::core::index_t* draw_order [[buffer(5)]],
        constant uint& entity_count [[buffer(6)]],
        texture2d<float, access::read_write> out_texture [[texture(0)]],
        uint2 gid [[thread_position_in_grid]])
{
    if (gid.x >= sc::display::kWidth || gid.y >= sc::display::kHeight)
        return;

    float4 out_color{out_texture.read(gid)};

    for (uint i{0}; i < entity_count; ++i) {
        const sc::core::index_t entity_idx{draw_order[i]};

        const auto entity_coord{float2(x_coords[entity_idx],
                y_coords[entity_idx] - z_coords[entity_idx])};
        const auto local_coord{int2(gid) - int2(floor(entity_coord))};

        if (local_coord.x < 0 ||
                static_cast<uint>(local_coord.x) >= sc::sprites::kWidth ||
                local_coord.y < 0 ||
                static_cast<uint>(local_coord.y) >= sc::sprites::kHeight)
            continue;

        constant sc::sprites::sprite& sprite{sprites[sprite_ids[entity_idx]]};
        const sc::sprites::packed_pixel pixel{
                sprite.pixels[local_coord.y][local_coord.x]};

        if (pixel.alpha == 0x00)
            continue;

        const float a{static_cast<float>(pixel.alpha) / 3.0f};
        const float4 sprite_color{unpack_color(sprite, pixel)};
        if (a < 1.0f) {
            out_color = (sprite_color * a) + (out_color * (1.0f - a));
            out_color.a = 1.0f;
        }
        else
            out_color = sprite_color;

        out_texture.write(out_color, gid);
    }
}
