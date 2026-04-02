#include "assets/sprite.hh"

#include <format>
#include <ostream>

std::ostream& operator<<(std::ostream& out, sc::assets::sprites::metadata meta)
{
    out << std::format(
            "BBox: ({:02}, {:02}), ({:02}, {:02}) | Origin: ({:02.2f}, "
            "{:02.2f}) | Encoding: {} | Palette: {} | Physics Type: {}\n",
            meta.bbox.min_u, meta.bbox.min_v, meta.bbox.max_u, meta.bbox.max_v,
            meta.origin_u, meta.origin_v,
            static_cast<unsigned>(meta.color_encoding), meta.palette_index,
            meta.physics_type);

    return out;
}
