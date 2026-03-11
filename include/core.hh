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
        using atlas_index_t = index_t;
        using packed_color_t = uint16_t;
        using packed_pixel_t = uint8_t;

        static SC_CONSTANT auto kAlignment{16u};

    } // namespace core

    namespace sprites {

        static SC_CONSTANT uint32_t kHeight{32u};
        static SC_CONSTANT uint32_t kWidth{32u};
        static SC_CONSTANT uint32_t kMaxPaletteSize{16u};

        static SC_CONSTANT uint64_t kAtlasMagicBytes{0x53414C5441204353};

        /**
         * @enum color_encoding
         * @brief Distribution of color channels across 16-bit packed integers.
         */
        enum class color_encoding : uint8_t {
            DEFAULT = 1, ///< R5G6B5
            WARM, ///< R6G5B5
            COOL, ///< R5G5B6
        };

        /**
         * @enum physics_type
         * @brief The type of physics that affects an entity.
         */
        enum class physics_type : uint8_t {
            NONE = 0,
            ACTOR,
            STATIC,
            SENSOR,
            PROJECTILE
        };

        /**
         * @union packed_pixel
         * @brief 8-bit packed index/metadata pixel.
         */
        union packed_pixel {
            core::packed_pixel_t data;
            struct {
                core::packed_pixel_t index : 4;
                core::packed_pixel_t alpha : 2;
                core::packed_pixel_t emission : 1;
                core::packed_pixel_t specular : 1;
            };
        };

    } // namespace sprites

    namespace assets {

        static SC_CONSTANT auto kCharacterAtlas{"assets/master.atlas"};
        static SC_CONSTANT auto kShaderLib{"assets/shader.metallib"};

    } // namespace assets

    namespace display {

        static SC_CONSTANT uint32_t kWidth{240u};
        static SC_CONSTANT uint32_t kHeight{160u};

        static SC_CONSTANT float kDefaultR{0.06f};
        static SC_CONSTANT float kDefaultG{0.22f};
        static SC_CONSTANT float kDefaultB{0.06f};

    } // namespace display

} // namespace sc
