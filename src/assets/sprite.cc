#include "assets/sprite.hh"

#include <format>
#include <ostream>

std::ostream& operator<<(std::ostream& out, sc::assets::sprites::metadata meta)
{
    out << std::format(
            "BBox: ({:02}, {:02}), ({:02}, {:02}) | Origin: ({:02.2f}, "
            "{:02.2f}) | Encoding: {} | Palette: {} | Physics Type: {}\n",
            meta.bbox.u_min, meta.bbox.v_min, meta.bbox.u_max, meta.bbox.v_max,
            meta.u_anchor, meta.v_anchor,
            static_cast<unsigned>(meta.color_encoding), meta.palette_index,
            meta.physics_type);

    return out;
}
