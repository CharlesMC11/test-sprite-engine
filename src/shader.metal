#include <metal_stdlib>

#include "core.hh"
#include "sprite.hh"

using namespace metal;

namespace sp = sc::sprites;

[[kernel]] void k_clear_screen(texture2d<float, access::read_write> out
        [[texture(0)]],
        uint2 gid [[thread_position_in_grid]])
{
    if (gid.x >= out.get_width() || gid.y >= out.get_height())
        return;

    constexpr auto bg_color{float4(sc::display::kDefaultR,
            sc::display::kDefaultG, sc::display::kDefaultB, 1.0f)};

    out.write(bg_color, gid);
}

inline float4 unpack_color(
        const sp::packed_color packed_color, const sp::color_encoding encoding)
{
    float r{1.0f}, g{1.0f}, b{1.0f};
    switch (encoding) {
    case sp::color_encoding::DEFAULT:
        r = static_cast<float>((packed_color >> 11) & 0x1F) / 31.0f;
        g = static_cast<float>((packed_color >> 5) & 0x3F) / 63.0f;
        b = static_cast<float>(packed_color & 0x1F) / 31.0f;
        break;
    case sp::color_encoding::WARM:
        r = static_cast<float>((packed_color >> 10) & 0x3F) / 63.0f;
        g = static_cast<float>((packed_color >> 5) & 0x1F) / 31.0f;
        b = static_cast<float>(packed_color & 0x1F) / 31.0f;
        break;
    case sp::color_encoding::COOL:
        r = static_cast<float>((packed_color >> 11) & 0x1F) / 31.0f;
        g = static_cast<float>((packed_color >> 5) & 0x1F) / 31.0f;
        b = static_cast<float>(packed_color & 0x3F) / 63.0f;
        break;
    default:
        break;
    }

    return float4(r, g, b, 1.0f);
}

[[kernel]] void k_draw_sprites(constant sp::palette* palettes [[buffer(0u)]],
        constant sp::sprite32* sprites [[buffer(1u)]],
        constant float* pos_x_ptr [[buffer(2u)]],
        constant float* pos_y_ptr [[buffer(3u)]],
        constant float* pos_z_ptr [[buffer(4u)]],
        constant sc::core::atlas_index* sprite_indices [[buffer(5u)]],
        constant sc::core::index_t* draw_order [[buffer(6u)]],
        constant uint& entity_count [[buffer(7u)]],
        texture2d<float, access::read_write> out [[texture(0u)]],
        uint2 gid [[thread_position_in_grid]])
{
    if (gid.x >= sc::display::kWidth || gid.y >= sc::display::kHeight)
        return;

    float4 out_color{out.read(gid)};

    for (uint i{0u}; i < entity_count; ++i) {
        const sc::core::index_t draw_idx{draw_order[i]};

        const auto entity_coord{float2(pos_x_ptr[draw_idx],
                pos_y_ptr[draw_idx] - pos_z_ptr[draw_idx])};

        const auto local_coord{int2(gid) - int2(floor(entity_coord))};

        if (local_coord.x < 0 ||
                static_cast<uint>(local_coord.x) >= sp::kWidth ||
                local_coord.y < 0 ||
                static_cast<uint>(local_coord.y) >= sp::kHeight)
            continue;

        constant sp::sprite32& sprite{sprites[sprite_indices[draw_idx]]};
        const sp::packed_pixel pixel{
                sprite.pixels[local_coord.y][local_coord.x]};

        const auto alpha_raw{(pixel & sp::kMaskAlpha) >> 4};
        if (alpha_raw == 0x00)
            continue;

        const sp::packed_color packed_color{
                palettes[sprite.meta.palette_index]
                        [pixel & sp::kMaskPaletteIndex]};

        const float4 color_normalized{unpack_color(packed_color,
                static_cast<sp::color_encoding>(sprite.meta.color_encoding))};

        const float alpha_normalized{static_cast<float>(alpha_raw) / 3.0f};
        if (alpha_normalized < 1.0f) {
            out_color.rgb = (color_normalized.rgb * alpha_normalized) +
                    (out_color.rgb * (1.0f - alpha_normalized));
        }
        else
            out_color.rgb = color_normalized.rgb;
    }

    out.write(out_color, gid);
}
