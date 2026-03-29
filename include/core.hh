/**
 * Source of truth for type aliases, constants, and enums.
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

        using index_t = unsigned;

        using atlas_index = index_t;

        using input_mask = unsigned;

        using physics_t = uint8_t;

        static SC_CONSTANT unsigned kNeonAlignment{16u};
        static SC_CONSTANT unsigned kCacheAlignment{128u};

#ifndef __METAL_VERSION__

        /**
         * Requirements for types to be safe to direct memory mapping.
         *
         * Type must be 16-byte aligned and follow Standard Layout to ensure the
         * CPU and GPU interpret the raw bytes identically.
         */
        template<typename T>
        concept mappable = alignof(T) % kNeonAlignment == 0 &&
                std::is_standard_layout_v<T> && !std::is_polymorphic_v<T>;

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

        static SC_CONSTANT unsigned kWidth{240u};
        static SC_CONSTANT unsigned kHeight{160u};

        static SC_CONSTANT float kDefaultR{0.50f};
        static SC_CONSTANT float kDefaultG{0.50f};
        static SC_CONSTANT float kDefaultB{0.50f};

        static SC_CONSTANT unsigned kTargetFPS{60u};

    } // namespace display

} // namespace sc
