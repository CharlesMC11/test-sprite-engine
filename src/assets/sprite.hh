#ifndef SC_ASSETS_SPRITE_HH
#define SC_ASSETS_SPRITE_HH

#ifndef __METAL_VERSION__
#include <cstdint>
#endif

#include "core/core.hh"
#include "graphics/graphics_types.hh"
#include "math/bbox.hh"

namespace sc::assets {

    namespace sprites {

        /**
         * A sprite’s metadata.
         *
         * Contains information regarding a sprite’s bounding box, pivot, color
         * encoding, and physics type.
         */
        struct alignas(core::kNeonAlignment) metadata final {
            geometry::bbox<> bbox;
            float origin_u{0.0f};
            float origin_v{0.0f};
            graphics::color_encoding color_encoding{0U};
            uint8_t palette_index{0U};
            core::physics_t physics_type{0U};
            uint8_t padding{0U};
        };

    } // namespace sprites

    /**
     * hardware-aware sprite definition.
     */
    template<unsigned Height = 32U, unsigned Width = Height>
    struct alignas(core::kNeonAlignment) sprite final {
        sprites::metadata meta;
        graphics::packed_pixel_t pixels[Height][Width]{0U}; // Row-major pixels
    };

    using sprite16 = sprite<16U>;
    using sprite32 = sprite<>;

    static_assert(sizeof(sprites::metadata) == core::kNeonAlignment,
            "Sprite metadata must be 16 B.");
    static_assert(sizeof(sprite16) == 272, "Sprite16 must be 272 B.");
    static_assert(sizeof(sprite<>) == 1'040, "Sprite32 must be 1,040 B.");

} // namespace sc::assets

#endif // SC_ASSETS_SPRITE_HH
