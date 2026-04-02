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

            if (std::abs(registry.vel_x_ptr()[a_idx]) < core::kEpsilon &&
                    std::abs(registry.vel_y_ptr()[a_idx]) < core::kEpsilon) {
                registry.new_x_ptr()[a_idx] = registry.pos_x_ptr()[a_idx];
                registry.new_y_ptr()[a_idx] = registry.pos_y_ptr()[a_idx];
                registry.new_z_ptr()[a_idx] = registry.pos_z_ptr()[a_idx];
                continue;
            }

            if (std::abs(registry.vel_y_ptr()[a_idx]) > core::kEpsilon)
                registry.draw_order_needs_sort = true;

            const float a_dx{registry.vel_x_ptr()[a_idx] * dt};
            const float a_dy{registry.vel_y_ptr()[a_idx] * dt};
            const float a_dz{registry.vel_z_ptr()[a_idx] * dt};

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

                        const assets::sprites::metadata& b_meta{
                                atlas.sprite32_span()
                                        [registry.sprite32_index_ptr()[b_idx]]
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

            apply_collision(registry, a_idx, collision, a_dx, a_dy, a_dz);
            if (collision.time < 1.0f)
                apply_slide(registry, a_idx, collision, dt);
        }
    }

    [[nodiscard]] static sweep_result sweep_aabb(
            const aabb& a, const aabb& b, const float dt)
    {
        sweep_result result;

        const float vel_x{(a.vel_x - b.vel_x) * dt};
        const float vel_y{(a.vel_y - b.vel_y) * dt};
        const float vel_z{(a.vel_z - b.vel_z) * dt};

        float entry_tx, exit_tx;
        if (std::abs(vel_x) < core::kEpsilon) {
            if (a.right < b.left || a.left > b.right)
                return result;

            entry_tx = -core::kInfinity;
            exit_tx = core::kInfinity;
        }
        else {
            const float in_x{
                    vel_x > 0.0f ? b.left - a.right : b.right - a.left};
            const float out_x{
                    vel_x > 0.0f ? b.right - a.left : b.left - a.right};

            entry_tx = in_x / vel_x;
            exit_tx = out_x / vel_x;
        }

        float entry_ty, exit_ty;
        if (std::abs(vel_y) < core::kEpsilon) {
            if (a.front < b.back || a.back > b.front)
                return result;

            entry_ty = -core::kInfinity;
            exit_ty = core::kInfinity;
        }
        else {
            const float in_y{
                    vel_y > 0.0f ? b.back - a.front : b.front - a.back};
            const float out_y{
                    vel_y > 0.0f ? b.front - a.back : b.back - a.front};

            entry_ty = in_y / vel_y;
            exit_ty = out_y / vel_y;
        }

        float entry_tz, exit_tz;
        if (std::abs(vel_z) < core::kEpsilon) {
            if (a.bottom > b.top || a.top < b.bottom)
                return result;

            entry_tz = -core::kInfinity;
            exit_tz = core::kInfinity;
        }
        else {
            const float in_z{
                    vel_z > 0.0f ? b.bottom - a.top : b.top - a.bottom};
            const float out_z{
                    vel_z > 0.0f ? b.top - a.bottom : b.bottom - a.top};

            entry_tz = in_z / vel_z;
            exit_tz = out_z / vel_z;
        }

        const float entry_t{std::max(entry_tx, std::max(entry_ty, entry_tz))};
        if (entry_t > std::min(exit_tx, std::min(exit_ty, exit_tz)) ||
                entry_t > 1.0f || entry_t < 0.0f ||
                (exit_tx < 0.0f && exit_ty < 0.0f && entry_tz < 0.0f))
            return result;

        result.time = entry_t;
        if (entry_tx >= entry_ty && entry_tx >= entry_tz) {
            result.normal_x = vel_x > 0.0f ? -1.0f : 1.0f;
            result.normal_y = 0.0f;
            result.normal_z = 0.0f;
        }
        else if (entry_ty >= entry_tx && entry_ty >= entry_tz) {
            result.normal_x = 0.0f;
            result.normal_y = vel_y > 0.0f ? -1.0f : 1.0f;
            result.normal_z = 0.0f;
        }
        else {
            result.normal_x = 0.0f;
            result.normal_y = 0.0f;
            result.normal_z = vel_z > 0.0f ? 1.0f : -1.0f;
        }

        return result;
    }

    static void apply_collision(entity_registry& registry,
            const core::index_t i, const sweep_result hit, const float dx,
            const float dy, const float dz)
    {
        const float padded_t{
                hit.time < 1.0f ? std::max(0.0f, hit.time - 0.1f) : 1.0f};

        registry.new_x_ptr()[i] = registry.pos_x_ptr()[i] + dx * padded_t;
        registry.new_y_ptr()[i] = registry.pos_y_ptr()[i] + dy * padded_t;
        registry.new_z_ptr()[i] = registry.pos_z_ptr()[i] + dz * padded_t;
    }

    static void apply_slide(entity_registry& registry, const core::index_t i,
            const sweep_result& hit, const float dt)
    {
        const float remain_t{1.0f - hit.time};

        const float dot{registry.vel_x_ptr()[i] * hit.normal_x +
                registry.vel_y_ptr()[i] * hit.normal_y +
                registry.vel_z_ptr()[i] * hit.normal_z};

        const float slide_x{registry.vel_x_ptr()[i] - dot * hit.normal_x};
        const float slide_y{registry.vel_y_ptr()[i] - dot * hit.normal_y};
        const float slide_z{registry.vel_z_ptr()[i] - dot * hit.normal_z};

        registry.new_x_ptr()[i] += slide_x * dt * remain_t;
        registry.new_y_ptr()[i] += slide_y * dt * remain_t;
        registry.new_z_ptr()[i] += slide_z * dt * remain_t;

        registry.vel_x_ptr()[i] = slide_x;
        registry.vel_y_ptr()[i] = slide_y;
        registry.vel_z_ptr()[i] = slide_z;
    }

} // namespace sc::physics
