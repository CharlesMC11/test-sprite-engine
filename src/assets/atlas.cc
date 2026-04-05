#include "assets/atlas.hh"

#include <format>
#include <ostream>

#include "core/core.hh"
#include "graphics/graphics_types.hh"

std::ostream& operator<<(std::ostream& out, const sc::assets::atlas& atlas)
{
    const sc::core::index_t palette_count{atlas.meta.palette_count};
    const sc::core::index_t sprite16_count{atlas.meta.sprite16_count};
    const sc::core::index_t sprite32_count{atlas.meta.sprite32_count};

    out << std::format(
            "Atlas: #Palette: {}, #Sprite16×16: {}, #Sprite32×32: {}\n\n",
            palette_count, sprite16_count, sprite32_count);

    for (sc::core::index_t i{0U}; i < palette_count; ++i)
        out << std::format("Palette {:02X}: ", i) << atlas.palette_span()[i]
            << '\n';
    out << '\n';

    for (sc::core::index_t i{0U}; i < sprite16_count; ++i)
        out << std::format("Sprite {:02}: 16×16 | ", i)
            << atlas.sprite16_span()[i];

    for (sc::core::index_t i{0U}; i < sprite32_count; ++i)
        out << std::format("Sprite {:02}: 32×32 | ", i)
            << atlas.sprite32_span()[i];

    return out;
}
