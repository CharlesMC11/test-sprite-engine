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
        for (core::index_t a_idx{0U}; a_idx < registry.count(); ++a_idx) {
            const assets::sprites::metadata a_meta{
                    atlas.sprite32_span()[registry.sprite_index_ptr()[a_idx]]
                            .meta};

            if (!core::any(
                        static_cast<type>(a_meta.physics_type) & type::ACTOR))
                continue;

            if (std::abs(registry.x_vel_ptr()[a_idx]) < core::kEpsilon &&
                    std::abs(registry.y_vel_ptr()[a_idx]) < core::kEpsilon) {
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

            const auto a_sweep_left{static_cast<int>(
                    std::min(a_aabb.left, a_aabb.left + a_dx))};
            const auto a_sweep_right{static_cast<int>(
                    std::max(a_aabb.right, a_aabb.right + a_dx))};

            const auto a_sweep_back{static_cast<int>(
                    std::min(a_aabb.back, a_aabb.back + a_dy))};
            const auto a_sweep_front{static_cast<int>(
                    std::max(a_aabb.front, a_aabb.front + a_dy))};

            const auto a_x_start{std::max(0, a_sweep_left / kCellSize - 1)};
            const auto a_x_end{
                    std::min(kColCount - 1, a_sweep_right / kCellSize + 1)};

            const auto a_y_start{std::max(0, a_sweep_back / kCellSize - 1)};
            const auto a_y_end{
                    std::min(kRowCount - 1, a_sweep_front / kCellSize + 1)};

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

            if (collision.time < 1.0f) {
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
        if (std::abs(x_vel) < core::kEpsilon) {
            if (a.right < b.left || a.left > b.right)
                return result;

            x_entry_time = -core::kInfinity;
            x_exit_time = core::kInfinity;
        }
        else {
            const float x_entry_pos{
                    x_vel > 0.0f ? b.left - a.right : b.right - a.left};
            const float x_exit_pos{
                    x_vel > 0.0f ? b.right - a.left : b.left - a.right};

            x_entry_time = x_entry_pos / x_vel;
            x_exit_time = x_exit_pos / x_vel;
        }

        float y_entry_time, y_exit_time;
        if (std::abs(y_vel) < core::kEpsilon) {
            if (a.front < b.back || a.back > b.front)
                return result;

            y_entry_time = -core::kInfinity;
            y_exit_time = core::kInfinity;
        }
        else {
            const float y_entry_pos{
                    y_vel > 0.0f ? b.back - a.front : b.front - a.back};
            const float y_exit_pos{
                    y_vel > 0.0f ? b.front - a.back : b.back - a.front};

            y_entry_time = y_entry_pos / y_vel;
            y_exit_time = y_exit_pos / y_vel;
        }

        float z_entry_time, z_exit_time;
        if (std::abs(z_vel) < core::kEpsilon) {
            if (a.bottom > b.top || a.top < b.bottom)
                return result;

            z_entry_time = -core::kInfinity;
            z_exit_time = core::kInfinity;
        }
        else {
            const float z_entry_pos{
                    z_vel > 0.0f ? b.bottom - a.top : b.top - a.bottom};
            const float z_exit_pos{
                    z_vel > 0.0f ? b.top - a.bottom : b.bottom - a.top};

            z_entry_time = z_entry_pos / z_vel;
            z_exit_time = z_exit_pos / z_vel;
        }

        const float entry_time{
                std::max(x_entry_time, std::max(y_entry_time, z_entry_time))};
        if (entry_time > std::min(x_exit_time,
                                 std::min(y_exit_time, z_exit_time)) ||
                entry_time > 1.0f || entry_time < 0.0f ||
                (x_exit_time < 0.0f && y_exit_time < 0.0f &&
                        z_entry_time < 0.0f))
            return result;

        result.time = entry_time;
        if (x_entry_time >= y_entry_time && x_entry_time >= z_entry_time) {
            result.normal_x = x_vel > 0.0f ? -1.0f : 1.0f;
            result.normal_y = 0.0f;
            result.normal_z = 0.0f;
        }
        else if (y_entry_time >= x_entry_time && y_entry_time >= z_entry_time) {
            result.normal_x = 0.0f;
            result.normal_y = y_vel > 0.0f ? -1.0f : 1.0f;
            result.normal_z = 0.0f;
        }
        else {
            result.normal_x = 0.0f;
            result.normal_y = 0.0f;
            result.normal_z = z_vel > 0.0f ? 1.0f : -1.0f;
        }

        return result;
    }

    static void apply_collision(entity_registry& registry,
            const core::index_t i, const sweep_result hit, const float dx,
            const float dy, const float dz)
    {
        const float padded_t{std::max(0.0f, hit.time - 0.1f)};

        registry.new_x_pos_ptr()[i] = registry.x_pos_ptr()[i] + dx * padded_t;
        registry.new_y_pos_ptr()[i] = registry.y_pos_ptr()[i] + dy * padded_t;
        registry.new_z_pos_ptr()[i] = registry.z_pos_ptr()[i] + dz * padded_t;
    }

    static void apply_slide(entity_registry& registry, const core::index_t i,
            const sweep_result hit, const float dt)
    {
        const float remain_t{1.0f - hit.time};

        const float dot{registry.x_vel_ptr()[i] * hit.normal_x +
                registry.y_vel_ptr()[i] * hit.normal_y +
                registry.z_vel_ptr()[i] * hit.normal_z};

        const float slide_x{registry.x_vel_ptr()[i] - dot * hit.normal_x};
        const float slide_y{registry.y_vel_ptr()[i] - dot * hit.normal_y};
        const float slide_z{registry.z_vel_ptr()[i] - dot * hit.normal_z};

        registry.new_x_pos_ptr()[i] += slide_x * dt * remain_t;
        registry.new_y_pos_ptr()[i] += slide_y * dt * remain_t;
        registry.new_z_pos_ptr()[i] += slide_z * dt * remain_t;

        registry.x_vel_ptr()[i] = slide_x;
        registry.y_vel_ptr()[i] = slide_y;
        registry.z_vel_ptr()[i] = slide_z;
    }

} // namespace sc::physics
