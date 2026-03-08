#pragma once

#ifdef __METAL_VERSION__
#define SC_CONSTANT constant constexpr
#else
#define SC_CONSTANT constexpr
#endif

namespace sc {

    SC_CONSTANT uint8_t PIXEL_INDEX_MASK{0x0F};
    SC_CONSTANT uint8_t PIXEL_ALPHA_MASK{0x30};
    SC_CONSTANT uint8_t PIXEL_GLOW_MASK{0x40};

    SC_CONSTANT uint32_t SPRITE_MAX_PALETTE_SIZE{16};
    SC_CONSTANT uint32_t SPRITE_HEIGHT{32};
    SC_CONSTANT uint32_t SPRITE_WIDTH{32};

    enum class color_encoding : uint8_t {
        DEFAULT = 1, // R5G6B5
        WARM, // R6G5B5
        COOL, // R5G5B6
    };

    using color = uint16_t;

    /// Bitmapping:
    /// - Index: Bits 0–3
    /// - Alpha: Bits 4–5
    /// - Glow: Bit 6
    /// - Reserved: Bit 7
    using pixel = uint8_t;

    struct alignas(16) sprite {
        uint8_t hb_min_x, hb_min_y;
        uint8_t hb_max_x, hb_max_y;
        uint8_t anchor_x, anchor_y;
        color_encoding encoding;
        color palette[SPRITE_MAX_PALETTE_SIZE];
        pixel pixels[SPRITE_HEIGHT * SPRITE_HEIGHT];
    };

} // namespace sc
