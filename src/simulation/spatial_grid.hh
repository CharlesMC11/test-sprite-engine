#ifndef SC_PHYSICS_SPATIAL_GRID_HH
#define SC_PHYSICS_SPATIAL_GRID_HH

#include <algorithm>
#include <cstddef>

#include "graphics/display_constants.hh"
#include "registry/entity_registry.hh"

namespace sc ::physics {

    static constexpr int kCellSize{16U};
    static constexpr int kColCount{display::kWidth / kCellSize};
    static constexpr int kRowCount{display::kHeight / kCellSize};
    static constexpr auto kTotalCells{
            static_cast<std::size_t>(kColCount) * kRowCount};

    struct alignas(core::kCacheAlignment) spatial_grid final {

        inline void update(entity_registry& registry) noexcept;

        core::index_t cell_heads[kTotalCells];

    private:
        [[nodiscard]] static constexpr core::index_t hash(
                float x, float y) noexcept;

        inline void clear() noexcept;
    };

    // Public methods

    inline void spatial_grid::update(entity_registry& registry) noexcept
    {
        clear();

        core::index_t* __restrict next_ptr{registry.next_in_cell_ptr()};
        for (std::size_t i{0UZ}; i < registry.count(); ++i) {
            const core::index_t cell_idx{
                    hash(registry.x_pos_ptr()[i], registry.y_pos_ptr()[i])};

            next_ptr[i] = cell_heads[cell_idx];
            cell_heads[cell_idx] = static_cast<core::index_t>(i);
        }
    }

    // Private helpers

    [[nodiscard]] constexpr core::index_t spatial_grid::hash(
            const float x, const float y) noexcept
    {
        const auto ix{static_cast<core::index_t>(std::clamp(x, 0.0f,
                              static_cast<float>(display::kWidth) - 1.0f)) /
                kCellSize};
        const auto iy{static_cast<core::index_t>(std::clamp(y, 0.0f,
                              static_cast<float>(display::kHeight) - 1.0f)) /
                kCellSize};

        return iy * kColCount + ix;
    }

    inline void spatial_grid::clear() noexcept
    {
        std::ranges::fill(std::begin(cell_heads), std::end(cell_heads),
                core::kInvalidIndex);
    }

} // namespace sc::physics

#endif // SC_PHYSICS_SPATIAL_GRID_HH
