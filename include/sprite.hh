/**
 * @file sprite.hh
 * @brief Core definitions and sprite data structures..
 */
#pragma once

#ifndef __METAL_VERSION__
#include <cstdint>
#endif

#include "core.hh"

namespace sc::sprites {

    SC_CONSTANT uint32_t kMaxPaletteSize{16};
    SC_CONSTANT uint32_t kHeight{32};
    SC_CONSTANT uint32_t kWidth{32};

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
    using packed_color = uint16_t;

    /**
     * @union packed_pixel
     * @brief 8-bit packed index/metadata pixel.
     */
    union packed_pixel {
        uint8_t data;
        struct {
            uint8_t index : 4;
            uint8_t alpha : 2;
            uint8_t emission : 1;
            uint8_t specular : 1;
        };
    };

    /**
     * @struct sprite
     * @brief A hardware-aware sprite definition.
     *
     * Uses 16-byte alignment to satisfy AArch64 SIMD and Metal address space
     * for constant sys.
     */
    struct alignas(sys::kAlignment) sprite final {
        uint8_t left, top, right, bottom; ///< Hitbox
        uint8_t anchor_x, anchor_y; ///< Local origin
        color_encoding encoding; ///< Channel packing used in palette
        uint8_t phys_type;
        packed_color palette[kMaxPaletteSize]; ///< 16-color LUT
        packed_pixel pixels[kHeight * kHeight]; ///< Row-major pixels
    };

} // namespace sc::sprites
