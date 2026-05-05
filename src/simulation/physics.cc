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

            if (std::abs(registry.y_vel_ptr()[a_idx]) > core::kEpsilon)
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

                        if (ab_collision.time < collision.time)
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

    [[nodiscard]] static sweep_result sweep_aabb(
            const aabb& a, const aabb& b, const float dt)
    {
        sweep_result result;

        const float x_vel{(a.x_vel - b.x_vel) * dt};
        const float y_vel{(a.y_vel - b.y_vel) * dt};
        const float z_vel{(a.z_vel - b.z_vel) * dt};

        float x_entry_time, x_exit_time;
        if (std::isless(std::abs(x_vel), core::kEpsilon)) {
            if (std::isless(a.east, b.west) || std::isgreater(a.west, b.east))
                return result;

            x_entry_time = -core::kInfinity;
            x_exit_time = core::kInfinity;
        }
        else {
            const float x_entry_pos{
                    !std::signbit(x_vel) ? b.west - a.east : b.east - a.west};
            const float x_exit_pos{
                    !std::signbit(x_vel) ? b.east - a.west : b.west - a.east};

            const float x_vel_inv{1 / x_vel};
            x_entry_time = x_entry_pos * x_vel_inv;
            x_exit_time = x_exit_pos * x_vel_inv;
        }

        float y_entry_time, y_exit_time;
        if (std::isless(std::abs(y_vel), core::kEpsilon)) {
            if (std::isless(a.south, b.north) ||
                    std::isgreater(a.north, b.south))
                return result;

            y_entry_time = -core::kInfinity;
            y_exit_time = core::kInfinity;
        }
        else {
            const float y_entry_pos{!std::signbit(y_vel) ? b.north - a.south
                                                         : b.south - a.north};
            const float y_exit_pos{!std::signbit(y_vel) ? b.south - a.north
                                                        : b.north - a.south};

            const float y_vel_inv{1 / y_vel};
            y_entry_time = y_entry_pos * y_vel_inv;
            y_exit_time = y_exit_pos * y_vel_inv;
        }

        float z_entry_time, z_exit_time;
        if (std::isless(std::abs(z_vel), core::kEpsilon)) {
            if (std::isgreater(a.nadir, b.zenith) ||
                    std::isless(a.zenith, b.nadir))
                return result;

            z_entry_time = -core::kInfinity;
            z_exit_time = core::kInfinity;
        }
        else {
            const float z_entry_pos{!std::signbit(z_vel) ? b.nadir - a.zenith
                                                         : b.zenith - a.nadir};
            const float z_exit_pos{!std::signbit(z_vel) ? b.zenith - a.nadir
                                                        : b.nadir - a.zenith};

            const float z_vel_inv{1 / z_vel};
            z_entry_time = z_entry_pos * z_vel_inv;
            z_exit_time = z_exit_pos * z_vel_inv;
        }

        const float entry_time{
                std::max({x_entry_time, y_entry_time, z_entry_time})};
        const float exit_time{
                std::min({x_exit_time, y_exit_time, z_exit_time})};
        const bool exit_times_are_neg{std::signbit(
                std::max({x_exit_time, y_exit_time, z_exit_time}))};
        if (entry_time > exit_time || std::isgreater(entry_time, 1.0f) ||
                std::signbit(entry_time) || exit_times_are_neg)
            return result;

        result.time = entry_time;
        if (std::isgreaterequal(x_entry_time, y_entry_time) &&
                std::isgreaterequal(x_entry_time, z_entry_time)) {
            result.normal_x = std::copysign(1.0f, -x_vel);
            result.normal_y = 0.0f;
            result.normal_z = 0.0f;
        }
        else if (std::isgreaterequal(y_entry_time, x_entry_time) &&
                std::isgreaterequal(y_entry_time, z_entry_time)) {
            result.normal_x = 0.0f;
            result.normal_y = std::copysign(1.0f, -y_vel);
            result.normal_z = 0.0f;
        }
        else {
            result.normal_x = 0.0f;
            result.normal_y = 0.0f;
            result.normal_z = std::copysign(1.0f, -z_vel);
        }

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
