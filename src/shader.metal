#include <metal_stdlib>

#include "constants.hpp"
#include "sprite.hpp"

[[kernel]] void render_sprite(constant sc::sprite& sprite [[buffer(0)]],
        constant float2& position [[buffer(1)]],
        metal::texture2d<float, metal::access::read_write> out_texture
        [[texture(0)]],
        uint2 gid [[thread_position_in_grid]])
{
    const auto screen_coord{gid + uint2(position)};

    const auto pixel{sprite.pixels[gid.y * sc::SPRITE_WIDTH + gid.x]};

    const ushort packed_color{sprite.palette[pixel.index]};
    float r{1.0}, g{1.0}, b{1.0};
    switch (sprite.encoding) {
    case sc::color_encoding::DEFAULT:
        r = ((packed_color >> 11) & 0x1F) / 31.0;
        g = ((packed_color >> 5) & 0x3F) / 63.0;
        b = (packed_color & 0x1F) / 31.0;
        break;
    case sc::color_encoding::WARM:
        r = ((packed_color >> 10) & 0x3F) / 63.0;
        g = ((packed_color >> 5) & 0x1F) / 31.0;
        b = (packed_color & 0x1F) / 31.0;
        break;
    case sc::color_encoding::COOL:
        r = ((packed_color >> 11) & 0x1F) / 31.0;
        g = ((packed_color >> 5) & 0x1F) / 31.0;
        b = (packed_color & 0x3F) / 63.0;
        break;
    default:
        break;
    }
    const float a{pixel.alpha / 3.0};

    const auto sprite_color{float4(r, g, b, a)};
    const float4 background{out_texture.read(screen_coord)};

    float4 out_color{(sprite_color * a) + (background * (1 - a))};
    out_color.a = 1.0;

    out_texture.write(out_color, screen_coord);
}

[[kernel]] void render_registry(constant sc::sprite* atlas [[buffer(0)]],
        constant float* instance_x [[buffer(1)]],
        constant float* instance_y [[buffer(2)]],
        constant uint64_t* sprite_ids [[buffer(3)]],
        metal::texture2d<float, metal::access::read_write> out_texture
        [[texture(0)]],
        uint2 gid [[thread_position_in_grid]],
        uint2 iid [[threadgroup_position_in_grid]])
{

    uint entity_idx{iid.x};
    float2 pos = float2(instance_x[entity_idx], instance_y[entity_idx]);
    uint64_t atlas_idx = sprite_ids[entity_idx];
    uint2 local_id = gid & 31;

    const auto entity_pos{
            float2(instance_x[entity_idx], instance_y[entity_idx])};
    const auto screen_coord{uint2(entity_pos) + local_id};

    if (screen_coord.x >= sc::ui::SCREEN_WIDTH ||
            screen_coord.y >= sc::ui::SCREEN_HEIGHT)
        return;

    constant sc::sprite& sprite{atlas[atlas_idx]};
    const auto pixel{sprite.pixels[local_id.y * sc::SPRITE_WIDTH + local_id.x]};

    const ushort packed_color{sprite.palette[pixel.index]};
    float r{1.0}, g{1.0}, b{1.0};
    switch (sprite.encoding) {
    case sc::color_encoding::DEFAULT:
        r = ((packed_color >> 11) & 0x1F) / 31.0;
        g = ((packed_color >> 5) & 0x3F) / 63.0;
        b = (packed_color & 0x1F) / 31.0;
        break;
    case sc::color_encoding::WARM:
        r = ((packed_color >> 10) & 0x3F) / 63.0;
        g = ((packed_color >> 5) & 0x1F) / 31.0;
        b = (packed_color & 0x1F) / 31.0;
        break;
    case sc::color_encoding::COOL:
        r = ((packed_color >> 11) & 0x1F) / 31.0;
        g = ((packed_color >> 5) & 0x1F) / 31.0;
        b = (packed_color & 0x3F) / 63.0;
        break;
    default:
        break;
    }
    const float a{pixel.alpha / 3.0};

    const auto sprite_color{float4(r, g, b, a)};
    const float4 background{out_texture.read(screen_coord)};

    float4 out_color{(sprite_color * a) + (background * (1 - a))};
    out_color.a = 1.0;

    out_texture.write(out_color, screen_coord);
}
