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
#include <numeric>
#include <type_traits>
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

        static SC_CONSTANT uint32_t kNeonAlignment{16u};
        static SC_CONSTANT uint32_t kCacheAlignment{128u};

#ifndef __METAL_VERSION__

        /**
         * @concept mappable
         * @brief Requirements for types to be safe to direct memory mapping.
         *
         * Type must be 16-byte aligned and follow Standard Layout to ensure the
         * CPU and GPU interpret the raw bytes identically.
         */
        template<typename T>
        concept mappable = alignof(T) % kNeonAlignment == 0 &&
                std::is_standard_layout_v<T> &&
                std::is_trivially_copyable_v<T> && !std::is_polymorphic_v<T>;

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
