#include "simulation/spatial_grid.hh"

#include <ostream>

#include "core/core.hh"

std::ostream& operator<<(
        std::ostream& out, const sc::physics::spatial_grid& grid)
{
    constexpr auto row_count{
            static_cast<sc::core::index_t>(sc::physics::kRowCount)};
    constexpr auto col_count{
            static_cast<sc::core::index_t>(sc::physics::kColCount)};

    for (sc::core::index_t y{0U}; y < row_count; ++y) {
        for (sc::core::index_t x{0U}; x < col_count; ++x)
            out << grid.cell_heads[y * col_count + x] << ' ';
        out << '\n';
    }

    return out;
}
