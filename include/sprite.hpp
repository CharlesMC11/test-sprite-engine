#pragma once

#define SC_SPRITE_MAX_PALETTE_SIZE 16
#define SC_SPRITE_HEIGHT 32
#define SC_SPRITE_WIDTH 32

namespace sc {

    enum class color_encoding : uint8_t {
        DEFAULT = 1, // R5G6B5
        WARM, // R6G5B5
        COOL, // R5G5B6
    };

    using color = uint16_t;

    struct pixel {
        uint8_t index : 4;
        uint8_t alpha : 2;
        uint8_t glow : 1;
        uint8_t reserved : 1;
    };

    struct alignas(16) sprite {
        uint8_t hb_min_x, hb_min_y;
        uint8_t hb_max_x, hb_max_y;
        uint8_t anchor_x, anchor_y;
        color_encoding encoding;
        uint8_t reserved;
        color palette[SC_SPRITE_MAX_PALETTE_SIZE];
        pixel pixels[SC_SPRITE_HEIGHT * SC_SPRITE_HEIGHT];
        uint64_t padding;
    };

} // namespace sc
