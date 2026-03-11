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

        static SC_CONSTANT uint8_t kMaskPaletteIndex{0x0F};
        static SC_CONSTANT uint8_t kMaskAlpha{0x30};
        static SC_CONSTANT uint8_t kMaskEmission{0x40};
        static SC_CONSTANT uint8_t kMaskSpecular{0x80};

        static SC_CONSTANT uint64_t kAtlasMagicBytes{0x53414C5441204353};

        /**
         * @enum color_encoding
         * @brief Distribution of color channels across 16-bit packed integers.
         */
        enum class color_encoding : uint8_t {
            DEFAULT = 1, ///< R5G6B5
            WARM, ///< R6G5B5
            COOL ///< R5G5B6
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
                core::packed_pixel_t index : 4u;
                core::packed_pixel_t alpha : 2u;
                core::packed_pixel_t emission : 1u;
                core::packed_pixel_t specular : 1u;
            };
        };

    } // namespace sprites

    namespace physics {

        static SC_CONSTANT float kGravity{9.8f};
        static SC_CONSTANT float kFixedTimestep{1.0f / 60.0f};
        static SC_CONSTANT float kMaxVelocity{500.0f};

    } // namespace physics

    namespace input {

        using mask_t = uint32_t;

        static SC_CONSTANT mask_t kUp{0x01};
        static SC_CONSTANT mask_t kDown{0x02};
        static SC_CONSTANT mask_t kLeft{0x04};
        static SC_CONSTANT mask_t kRight{0x08};

    } // namespace input

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

        static SC_CONSTANT uint32_t kTargetFPS{60u};

    } // namespace display

} // namespace sc
