#include <metal_stdlib>

#include "assets/sprite.hh"
#include "core/core.hh"
#include "graphics/display_constants.hh"
#include "graphics/graphics_types.hh"
#include "graphics/render_constants.hh"

using namespace metal;

[[kernel]] void k_clear_screen(texture2d<float, access::read_write> out
        [[texture(0U)]],
        uint2 gid [[thread_position_in_grid]])
{
    if (gid.x >= out.get_width() || gid.y >= out.get_height())
        return;

    constexpr auto bg_color{float4(sc::display::kDefaultR,
            sc::display::kDefaultG, sc::display::kDefaultB, 1.0f)};

    out.write(bg_color, gid);
}

inline float4 unpack_color(const sc::graphics::packed_color_t packed_color,
        const sc::graphics::color_encoding encoding)
{
    float r{1.0f};
    float g{1.0f};
    float b{1.0f};

    constexpr sc::graphics::packed_color_t k6BitMask{0x3FU};
    constexpr auto k6BitNorm{1.0f / static_cast<float>(k6BitMask)};
    constexpr sc::graphics::packed_color_t k5BitMask{0x1FU};
    constexpr auto k5BitNorm{1.0f / static_cast<float>(k5BitMask)};

    using enum sc::graphics::color_encoding;
    switch (encoding) {
    case neutral:
        r = static_cast<float>((packed_color >> 11) & k5BitMask) * k5BitNorm;
        g = static_cast<float>((packed_color >> 5) & k6BitMask) * k6BitNorm;
        b = static_cast<float>(packed_color & k5BitMask) * k5BitNorm;
        break;
    case warm:
        r = static_cast<float>((packed_color >> 10) & k6BitMask) * k6BitNorm;
        g = static_cast<float>((packed_color >> 5) & k5BitMask) * k5BitNorm;
        b = static_cast<float>(packed_color & k5BitMask) * k5BitNorm;
        break;
    case cool:
        r = static_cast<float>((packed_color >> 11) & k5BitMask) * k5BitNorm;
        g = static_cast<float>((packed_color >> 6) & k5BitMask) * k5BitNorm;
        b = static_cast<float>(packed_color & k6BitMask) * k6BitNorm;
        break;
    default:
        break;
    }

    return float4(r, g, b, 1.0f);
}

[[kernel]] void k_draw_sprites(constant sc::graphics::palette* palettes
        [[buffer(sc::render::BUFFER_INDEX_PALETTES)]],
        constant sc::assets::sprite32* sprites
        [[buffer(sc::render::BUFFER_INDEX_SPRITES)]],
        constant float* pos_x_ptr
        [[buffer(sc::render::BUFFER_INDEX_X_POSITIONS)]],
        constant float* pos_y_ptr
        [[buffer(sc::render::BUFFER_INDEX_Y_POSITIONS)]],
        constant float* pos_z_ptr
        [[buffer(sc::render::BUFFER_INDEX_Z_POSITIONS)]],
        constant sc::core::index_t* atlas_indices
        [[buffer(sc::render::BUFFER_INDEX_ATLAS_INDICES)]],
        constant sc::core::index_t* draw_order
        [[buffer(sc::render::BUFFER_INDEX_DRAW_ORDER)]],
        constant uint& entity_count
        [[buffer(sc::render::BUFFER_INDEX_ENTITY_COUNT)]],
        texture2d<float, access::read_write> out [[texture(0U)]],
        uint2 gid [[thread_position_in_grid]])
{
    if (gid.x >= sc::display::kWidth || gid.y >= sc::display::kHeight)
        return;

    float4 out_color{out.read(gid)};

    constexpr auto kAlphaNorm{1.0f / 3.0f};

    for (auto i{0U}; i < entity_count; ++i) {
        const sc::core::index_t draw_idx{draw_order[i]};

        const auto entity_coord{float2(pos_x_ptr[draw_idx],
                pos_y_ptr[draw_idx] - pos_z_ptr[draw_idx])};

        const auto local_coord{int2(gid) - int2(floor(entity_coord))};

        if (local_coord.x < 0 || static_cast<uint>(local_coord.x) >= 32U ||
                local_coord.y < 0 || static_cast<uint>(local_coord.y) >= 32U)
            continue;

        constant sc::assets::sprite32& sprite{sprites[atlas_indices[draw_idx]]};
        const sc::graphics::packed_pixel_t pixel{sprite.pixels[local_coord.y *
                        sc::assets::sprites::kDefaultSize +
                local_coord.x]};

        const auto alpha_raw{(pixel & sc::graphics::kMaskAlpha) >> 4};
        if (alpha_raw == 0x00U)
            continue;

        const sc::graphics::packed_color_t packed_color{
                palettes[sprite.meta.palette_index]
                        .colors[pixel & sc::graphics::kMaskPaletteIndex]};

        const float4 color_normalized{
                unpack_color(packed_color, sprite.meta.color_encoding)};

        const float alpha_normalized{
                static_cast<float>(alpha_raw) * kAlphaNorm};
        if (alpha_normalized < 1.0f) {
            out_color.rgb = (color_normalized.rgb * alpha_normalized) +
                    (out_color.rgb * (1.0f - alpha_normalized));
        }
        else
            out_color.rgb = color_normalized.rgb;
    }

    out.write(out_color, gid);
}
