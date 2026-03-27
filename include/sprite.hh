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

    static SC_CONSTANT uint32_t kMaxPaletteSize{16u};
    static SC_CONSTANT uint32_t kHeight{32u};
    static SC_CONSTANT uint32_t kWidth{32u};

    static SC_CONSTANT core::packed_pixel_t kMaskPaletteIndex{0x0F};
    static SC_CONSTANT core::packed_pixel_t kMaskAlpha{0x30};
    static SC_CONSTANT core::packed_pixel_t kMaskEmission{0x40};
    static SC_CONSTANT core::packed_pixel_t kMaskSpecular{0x80};

    /**
     * @enum color_encoding
     * @brief Distribution of color channels across 16-bit packed integers.
     */
    enum class color_encoding : uint8_t {
        DEFAULT = 1u, ///< R5G6B5
        WARM, ///< R6G5B5
        COOL ///< R5G5B6
    };

    /**
     * @union packed_pixel
     * @brief 8-bit packed index/metadata pixel.
     */
    union packed_pixel {
        core::packed_pixel_t data;
        struct {
            core::packed_pixel_t index : 4u;
            core::packed_pixel_t alpha : 2u;
            core::packed_pixel_t emission : 1u;
            core::packed_pixel_t specular : 1u;
        };
    };

    /**
     * @struct metadata
     * @brief A sprite’s metadata.
     *
     * Contains information regarding a sprite’s bounding box, anchors, color
     * encoding, and physics type.
     */
    struct alignas(core::kNeonAlignment) metadata final {
        geometry::bbox<uint8_t> bbox;
        float anchor_x, anchor_y;
        uint8_t color_encoding;
        uint8_t palette_index;
        uint8_t physics;
        uint8_t padding;
    };

    /**
     * @struct sprite
     * @brief A hardware-aware sprite definition.
     *
     * Uses 16-byte alignment to satisfy AArch64 SIMD and Metal address space
     * for constant sys.
     */
    struct alignas(core::kAlignment) sprite final {
        metadata metadata;
        core::packed_color_t palette[kMaxPaletteSize]; ///< 16-color LUT
        packed_pixel pixels[kHeight][kWidth]; ///< Row-major pixels
    };

    static_assert(sizeof(metadata) == 16, "Metadata must be 16 B.");
    static_assert(sizeof(sprite) == 1'072, "Sprite must be exactly 1,072 B.");

} // namespace sc::sprites
