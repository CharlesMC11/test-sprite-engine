#include "graphics/graphics_types.hh"

#include <format>
#include <ostream>

std::ostream& operator<<(
        std::ostream& out, const sc::graphics::palette& palette)
{
    for (const auto& c: palette.colors)
        out << std::format("[{:04X}] ", c);

    return out;
}
