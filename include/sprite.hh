/**
 * @file sprite.hh
 * @brief Core definitions and sprite data structures.
 */
#pragma once

#ifndef __METAL_VERSION__
#include <cstdint>
#endif

#include "bbox.hh"
#include "core.hh"

namespace sc::sprites {

    static SC_CONSTANT unsigned kMaxPaletteSize{16u};
    static SC_CONSTANT unsigned kHeight{32u};
    static SC_CONSTANT unsigned kWidth{32u};

    using packed_color = uint16_t;
    using palette = packed_color[kMaxPaletteSize];

    /**
     * @enum color_encoding
     * @brief Distribution of color channels across a 16-bit packed integer.
     */
    enum class color_encoding {
        DEFAULT = 0u, // R5G6B5
        WARM, // R6G5B5
        COOL // R5G5B6
    };

    using packed_pixel = uint8_t; // [S][E][AA][IIII]
    static SC_CONSTANT packed_pixel kMaskPaletteIndex{0x0F};
    static SC_CONSTANT packed_pixel kMaskAlpha{0x30};
    static SC_CONSTANT packed_pixel kMaskEmission{0x40};
    static SC_CONSTANT packed_pixel kMaskSpecular{0x80};

    /**
     * @struct metadata
     * @brief A sprite’s metadata.
     *
     * Contains information regarding a sprite’s bounding box, pivot, color
     * encoding, and physics type.
     */
    struct alignas(core::kNeonAlignment) metadata final {
        geometry::bbox<> bbox;
        float origin_u, origin_v;
        uint8_t color_encoding;
        uint8_t palette_index;
        uint8_t physics_type;
        uint8_t padding;
    };

    /**
     * @struct sprite
     * @brief A hardware-aware sprite definition.
     */
    template<unsigned Height, unsigned Width = Height>
    struct alignas(core::kNeonAlignment) sprite final {
        metadata meta;
        packed_pixel pixels[Height][Width]; // Row-major pixels
    };

    using sprite8 = sprite<8u>;
    using sprite16 = sprite<16u>;
    using sprite32 = sprite<32u>;
    using sprite64 = sprite<64u>;

    static_assert(
            sizeof(metadata) == core::kNeonAlignment, "Metadata must be 16 B.");
    static_assert(sizeof(sprite16) == 272, "Sprite16 must be 272 B.");
    static_assert(sizeof(sprite32) == 1'040, "Sprite32 must be 1,040 B.");

} // namespace sc::sprites
