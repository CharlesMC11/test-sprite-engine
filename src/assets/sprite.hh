#ifndef SC_ASSETS_SPRITE_HH
#define SC_ASSETS_SPRITE_HH

#ifndef __METAL_VERSION__
#include <cstdint>
#include <format>
#include <ostream>
#endif

#include "core/core.hh"
#include "graphics/graphics_types.hh"
#include "math/bbox.hh"

namespace sc::assets {

    namespace sprites {

        inline SC_CONSTEXPR uint32_t kDefaultSize{32U};

        /**
         * A sprite’s metadata.
         *
         * Contains information regarding a sprite’s bounding box, pivot, color
         * encoding, and physics type.
         */
        struct alignas(core::kNeonAlignment) metadata final {
            geometry::bbox<> bbox;
            float u_anchor{0.0f};
            float v_anchor{0.0f};
            uint8_t depth{0U};
            core::physics_t physics_type{0U};
            graphics::color_encoding color_encoding{0U};
            uint8_t palette_index{0U};
        };

    } // namespace sprites

    /**
     * hardware-aware sprite definition.
     */
    template<unsigned Width = sprites::kDefaultSize, unsigned Height = Width>
    struct alignas(core::kNeonAlignment) sprite final {
        sprites::metadata meta;
        graphics::packed_pixel_t pixels[Height * Width]{0U}; // Row-major pixels
    };

    using sprite16 = sprite<16U>;
    using sprite32 = sprite<>;

    static_assert(sizeof(sprites::metadata) == core::kNeonAlignment,
            "Sprite metadata must be 16 B.");
    static_assert(sizeof(sprite16) == 272, "Sprite16 must be 272 B.");
    static_assert(sizeof(sprite<>) == 1'040, "Sprite32 must be 1,040 B.");

} // namespace sc::assets

#ifndef __METAL_VERSION__

std::ostream& operator<<(std::ostream&, sc::assets::sprites::metadata);

template<unsigned Height, unsigned Width>
std::ostream& operator<<(
        std::ostream& out, const sc::assets::sprite<Height, Width>& sprite)
{
    out << sprite.meta;

    constexpr auto c{"+ "};

    for (std::size_t i{0UZ}; i < Width + 2UZ; ++i)
        out << c;
    out << '\n';

    for (std::size_t y{0UZ}; y < Height; ++y) {
        out << c;
        for (std::size_t x{0UZ}; x < Width; ++x) {
            const auto p{sprite.pixels[y * Width + x]};

            out << ((p & sc::graphics::kMaskAlpha) > 0x00U
                            ? std::format("{:X} ",
                                      p & sc::graphics::kMaskPaletteIndex)
                            : "  ");
        }
        out << c << '\n';
    }
    for (std::size_t i{0U}; i < Width + 2UZ; ++i)
        out << c;
    out << '\n';

    return out;
}

#endif //__METAL_VERSION__

#endif // SC_ASSETS_SPRITE_HH
