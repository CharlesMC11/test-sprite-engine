/**
 * @file sprite.hpp
 * @brief Core definitions and sprite data structures..
 */
#pragma once

#ifdef __METAL_VERSION__
#define SC_CONSTANT constant constexpr
#else
#define SC_CONSTANT constexpr
#include <cstdint>
#endif

namespace sc {

    /// \brief Bits 0–3: Palette index
    SC_CONSTANT uint8_t PIXEL_INDEX_MASK{0x0F};

    /// \brief Bits 4–5: 2-bit transparency
    SC_CONSTANT uint8_t PIXEL_ALPHA_MASK{0x30};

    /// \brief Bit 6: Glow flag
    SC_CONSTANT uint8_t PIXEL_GLOW_MASK{0x40};

    SC_CONSTANT uint32_t SPRITE_MAX_PALETTE_SIZE{16};
    SC_CONSTANT uint32_t SPRITE_HEIGHT{32};
    SC_CONSTANT uint32_t SPRITE_WIDTH{32};

    /**
     * @enum color_encoding
     * @brief Distribution of color channels across 16-bit packed integers.
     */
    enum class color_encoding : uint8_t {
        DEFAULT = 1, ///< R5G6B5
        WARM, ///< R6G5B5
        COOL, ///< R5G5B6
    };

    /// \brief 16-bit–packed color, encoding determined by `color_encoding`.
    using color = uint16_t;

    /// \brief 8-bit packed index/metadata pixel. See `PIXEL_` masks for
    /// mapping.
    using pixel = uint8_t;

    /**
     * @struct sprite
     * @brief A hardware-aware sprite definition.
     *
     * Uses 16-byte alignment to satisfy AArch64 SIMD and Metal address space
     * for constant memory.
     */
    struct alignas(16) sprite {
        uint8_t hb_min_x, hb_min_y; ///< Hitbox minimum bounds (top-left)
        uint8_t hb_max_x, hb_max_y; ///< Hitbox maximum bounds (bottom-right)
        uint8_t anchor_x, anchor_y; ///< Local origin
        color_encoding encoding; ///< Channel packing used in palette
        color palette[SPRITE_MAX_PALETTE_SIZE]; ///< 16-color LUT
        pixel pixels[SPRITE_HEIGHT * SPRITE_HEIGHT]; ///< Row-major pixel data
    };

} // namespace sc
