/**
 * @file core.hh
 * @brief Source of truth for type aliases, constants, and enums.
 */
#pragma once

#ifdef __METAL_VERSION__
#define SC_CONSTANT constant constexpr
#else
#define SC_CONSTANT constexpr
#include <cstdint>
#endif

namespace sc {

    namespace core {

        using index_t = uint32_t;

        using packed_color_t = uint16_t; ///< Distribution is `color_encoding`
        using packed_pixel_t = uint8_t;

        using atlas_index_t = index_t;
        using atlas_magic_t = uint64_t;

        using input_mask_t = uint32_t;

        using physics_t = uint8_t;

        static SC_CONSTANT auto kAlignment{16u};

#ifndef __METAL_VERSION__

        using float_limits = std::numeric_limits<float>;

        static constexpr float kEpsilon{float_limits::epsilon()};
        static constexpr float kInfinity{float_limits::infinity()};

#endif

    } // namespace core

    namespace assets {

        static SC_CONSTANT auto kCharacterAtlas{"assets/characters.atlas"};
        static SC_CONSTANT auto kShaderLib{"assets/shader.metallib"};

    } // namespace assets

    namespace display {

        static SC_CONSTANT uint32_t kWidth{240u};
        static SC_CONSTANT uint32_t kHeight{160u};

        static SC_CONSTANT float kDefaultR{0.06f};
        static SC_CONSTANT float kDefaultG{0.22f};
        static SC_CONSTANT float kDefaultB{0.06f};

        static SC_CONSTANT uint32_t kTargetFPS{60u};

    } // namespace display

} // namespace sc
