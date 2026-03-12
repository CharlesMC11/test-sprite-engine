/**
 * @file sprite.hh
 * @brief Core definitions and sprite data structures.
 */
#pragma once

#ifndef __METAL_VERSION__
#include <cstdint>
#endif

#include "core.hh"

namespace sc::sprites {

    /**
     * @struct sprite
     * @brief A hardware-aware sprite definition.
     *
     * Uses 16-byte alignment to satisfy AArch64 SIMD and Metal address space
     * for constant sys.
     */
    struct alignas(core::kAlignment) sprite final {
        uint8_t left, top, right, bottom; ///< Hitbox
        uint8_t anchor_x, anchor_y; ///< Local origin
        color_encoding encoding; ///< Channel packing used in palette
        physics_type physics;
        core::packed_color_t palette[kMaxPaletteSize]; ///< 16-color LUT
        packed_pixel pixels[kHeight][kWidth]; ///< Row-major pixels
    };

} // namespace sc::sprites
