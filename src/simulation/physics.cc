#include "simulation/physics.hh"

#include <cmath>

#include "core/core.hh"
#include "simulation/physics_types.hh"
#include "simulation/spatial_grid.hh"

namespace sc::physics {
    [[nodiscard]] static sweep_result sweep_aabb(
            const aabb& a, const aabb& b, float dt);

    static void apply_collision(entity_registry& registry, core::index_t i,
            sweep_result hit, float dx, float dy, float dz);

    static void apply_slide(entity_registry& registry, core::index_t i,
            sweep_result hit, float dt);

    void resolve_entity_collisions(entity_registry& registry,
            const spatial_grid& grid, const assets::atlas& atlas,
            const float dt)
    {
        const auto entity_count{static_cast<core::index_t>(registry.count())};
        for (core::index_t a_idx{0U}; a_idx < entity_count; ++a_idx) {
            const assets::sprites::metadata a_meta{
                    atlas.sprite32_span()[registry.sprite_index_ptr()[a_idx]]
                            .meta};

            if (!core::any(
                        static_cast<type>(a_meta.physics_type) & type::ACTOR))
                continue;

            if (std::isless(std::abs(registry.x_vel_ptr()[a_idx]),
                        core::kEpsilon) &&
                    std::isless(std::abs(registry.y_vel_ptr()[a_idx]),
                            core::kEpsilon)) {
                registry.new_x_pos_ptr()[a_idx] = registry.x_pos_ptr()[a_idx];
                registry.new_y_pos_ptr()[a_idx] = registry.y_pos_ptr()[a_idx];
                registry.new_z_pos_ptr()[a_idx] = registry.z_pos_ptr()[a_idx];
                continue;
            }

            if (std::isgreater(
                        std::abs(registry.y_vel_ptr()[a_idx]), core::kEpsilon))
                registry.draw_order_needs_sort = true;

            const float a_dx{registry.new_x_pos_ptr()[a_idx] -
                    registry.x_pos_ptr()[a_idx]};
            const float a_dy{registry.new_y_pos_ptr()[a_idx] -
                    registry.y_pos_ptr()[a_idx]};
            const float a_dz{registry.new_z_pos_ptr()[a_idx] -
                    registry.z_pos_ptr()[a_idx]};

            const auto a_aabb{aabb::from_registry(registry, a_idx, a_meta)};

            const auto a_sweep_west{static_cast<int>(
                    std::min(a_aabb.west, a_aabb.west + a_dx))};
            const auto a_sweep_east{static_cast<int>(
                    std::max(a_aabb.east, a_aabb.east + a_dx))};

            const auto a_sweep_north{static_cast<int>(
                    std::min(a_aabb.north, a_aabb.north + a_dy))};
            const auto a_sweep_south{static_cast<int>(
                    std::max(a_aabb.south, a_aabb.south + a_dy))};

            const int a_x_start{std::max(0, a_sweep_west / kCellSize - 1)};
            const int a_x_end{
                    std::min(kColCount - 1, a_sweep_east / kCellSize + 1)};

            const int a_y_start{std::max(0, a_sweep_north / kCellSize - 1)};
            const int a_y_end{
                    std::min(kRowCount - 1, a_sweep_south / kCellSize + 1)};

            sweep_result collision;

            for (int cy{a_y_start}; cy <= a_y_end; ++cy) {
                for (int cx{a_x_start}; cx <= a_x_end; ++cx) {
                    core::index_t b_idx{grid.cell_heads[cy * kColCount + cx]};

                    while (b_idx != core::kInvalidIndex) {
                        if (a_idx == b_idx) {
                            b_idx = registry.next_in_cell_ptr()[b_idx];
                            continue;
                        }

                        const assets::sprites::metadata b_meta{
                                atlas.sprite32_span()
                                        [registry.sprite_index_ptr()[b_idx]]
                                                .meta};

                        if (core::any(static_cast<type>(b_meta.physics_type) &
                                    type::NONE)) {
                            b_idx = registry.next_in_cell_ptr()[b_idx];
                            continue;
                        }

                        const auto b_aabb{
                                aabb::from_registry(registry, b_idx, b_meta)};

                        const sweep_result ab_collision{
                                sweep_aabb(a_aabb, b_aabb, dt)};

                        if (std::isless(ab_collision.time, collision.time))
                            collision = ab_collision;

                        b_idx = registry.next_in_cell_ptr()[b_idx];
                    }
                }
            }

            if (std::isless(collision.time, 1.0f)) {
                apply_collision(registry, a_idx, collision, a_dx, a_dy, a_dz);
                apply_slide(registry, a_idx, collision, dt);
            }
        }
    }

    [[nodiscard]] static std::pair<float, float> calculate_entry_exit_times(
            const float vel, const float a_min, const float a_max,
            const float b_min, const float b_max)
    {
        float entry_time, exit_time;
        if (std::isless(std::abs(vel), core::kEpsilon)) {
            if (std::isless(a_max, b_min) || std::isgreater(a_min, b_max))
                return {core::kNaN, core::kNaN};

            entry_time = -core::kInfinity;
            exit_time = core::kInfinity;
        }
        else {
            const float entry_pos{
                    !std::signbit(vel) ? b_min - a_max : b_max - a_min};
            const float exit_pos{
                    !std::signbit(vel) ? b_max - a_min : b_min - a_max};

            const float vel_inv{1.0f / vel};
            entry_time = entry_pos * vel_inv;
            exit_time = exit_pos * vel_inv;
        }

        return {entry_time, exit_time};
    }

    [[nodiscard]] static sweep_result sweep_aabb(
            const aabb& a, const aabb& b, const float dt)
    {
        sweep_result result;

        const float x_vel{(a.x_vel - b.x_vel) * dt};
        const auto [x_entry_time, x_exit_time]{calculate_entry_exit_times(
                x_vel, a.west, a.east, b.west, b.east)};
        if (std::isnan(x_entry_time))
            return result;

        const float y_vel{(a.y_vel - b.y_vel) * dt};
        const auto [y_entry_time, y_exit_time]{calculate_entry_exit_times(
                y_vel, a.north, a.south, b.north, b.south)};
        if (std::isnan(y_entry_time))
            return result;

        const float z_vel{(a.z_vel - b.z_vel) * dt};
        const auto [z_entry_time, z_exit_time]{calculate_entry_exit_times(
                z_vel, a.nadir, a.zenith, b.nadir, b.zenith)};
        if (std::isnan(z_entry_time))
            return result;

        const float entry_time{
                std::max({x_entry_time, y_entry_time, z_entry_time})};
        const float exit_time{
                std::min({x_exit_time, y_exit_time, z_exit_time})};
        if (std::signbit(exit_time) || entry_time > exit_time ||
                std::isgreater(entry_time, 1.0f))
            return result;

        result.time = entry_time;

        const bool x_is_max{std::isgreaterequal(x_entry_time, y_entry_time) &&
                std::isgreaterequal(x_entry_time, z_entry_time)};
        const bool y_is_max{
                !x_is_max && std::isgreaterequal(y_entry_time, z_entry_time)};

        result.normal_x = x_is_max ? std::copysign(1.0f, -x_vel) : 0.0f;
        result.normal_y = y_is_max ? std::copysign(1.0f, -y_vel) : 0.0f;
        result.normal_z =
                !x_is_max && !y_is_max ? std::copysign(1.0f, -z_vel) : 0.0f;

        return result;
    }

    static void apply_collision(entity_registry& registry,
            const core::index_t i, const sweep_result hit, const float dx,
            const float dy, const float dz)
    {
        const float padded_t{std::clamp(hit.time - 0.01f, 0.0f, 1.0f)};

        registry.new_x_pos_ptr()[i] =
                std::fma(dx, padded_t, registry.x_pos_ptr()[i]);
        registry.new_y_pos_ptr()[i] =
                std::fma(dy, padded_t, registry.y_pos_ptr()[i]);
        registry.new_z_pos_ptr()[i] =
                std::fma(dz, padded_t, registry.z_pos_ptr()[i]);
    }

    static void apply_slide(entity_registry& registry, const core::index_t i,
            const sweep_result hit, const float dt)
    {
        const float remain_t{1.0f - hit.time};

        const float dot_neg{-std::fma(registry.x_vel_ptr()[i], hit.normal_x,
                std::fma(registry.y_vel_ptr()[i], hit.normal_y,
                        registry.z_vel_ptr()[i] * hit.normal_z))};

        const float slide_x{
                std::fma(dot_neg, hit.normal_x, registry.x_vel_ptr()[i])};
        const float slide_y{
                std::fma(dot_neg, hit.normal_y, registry.y_vel_ptr()[i])};
        const float slide_z{
                std::fma(dot_neg, hit.normal_z, registry.z_vel_ptr()[i])};

        const float displacement_factor{dt * remain_t};
        registry.new_x_pos_ptr()[i] = std::fma(
                slide_x, displacement_factor, registry.new_x_pos_ptr()[i]);
        registry.new_y_pos_ptr()[i] = std::fma(
                slide_y, displacement_factor, registry.new_y_pos_ptr()[i]);
        registry.new_z_pos_ptr()[i] = std::fma(
                slide_z, displacement_factor, registry.new_z_pos_ptr()[i]);

        registry.x_vel_ptr()[i] = slide_x;
        registry.y_vel_ptr()[i] = slide_y;
        registry.z_vel_ptr()[i] = slide_z;
    }

} // namespace sc::physics
