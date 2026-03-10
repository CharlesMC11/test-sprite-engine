/**
 * @file sprite.hpp
 * @brief Core definitions and sprite data structures..
 */
#pragma once

#ifndef __METAL_VERSION__
#include <cstdint>
#endif

#include "constants.hpp"

namespace sc {

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

    /**
     * @union pixel_unit
     * @brief 8-bit packed index/metadata pixel.
     */
    union pixel_unit {
        uint8_t buffer;
        struct {
            uint8_t index : 4;
            uint8_t alpha : 2;
            uint8_t glow : 1;
        };
    };

    /**
     * @struct sprite
     * @brief A hardware-aware sprite definition.
     *
     * Uses 16-byte alignment to satisfy AArch64 SIMD and Metal address space
     * for constant sys.
     */
    struct alignas(sys::ALIGNMENT) sprite final {
        uint8_t hb_left, hb_top; ///< Hitbox minimum bounds (top-left)
        uint8_t hb_right, hb_bottom; ///< Hitbox maximum bounds (bottom-right)
        uint8_t anchor_x, anchor_y; ///< Local origin
        color_encoding encoding; ///< Channel packing used in palette
        color palette[SPRITE_MAX_PALETTE_SIZE]; ///< 16-color LUT
        pixel_unit pixels[SPRITE_HEIGHT * SPRITE_HEIGHT]; ///< Row-major pixels
    };

} // namespace sc
