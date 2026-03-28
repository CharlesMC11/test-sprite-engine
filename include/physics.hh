/**
 * @file physics.hh
 * @brief
 */
#pragma once

#include "atlas.hh"
#include "bbox.hh"
#include "core.hh"
#include "scene_registry.hh"
#include "sprite.hh"

namespace sc::physics {

    static constexpr float kGravity{9.8f};
    static constexpr float kFixedTimestep{1.0f / 120.0f};
    static constexpr float kMaxVelocity{500.0f};

    static constexpr float kYCollisionDistance{8.0f};

    struct aabb {
        explicit aabb(const float x, const float y, const float z,
                const float vx, const float vy, const float vz,
                const geometry::bbox<float>& bbox) noexcept
            : left{x + bbox.min_u}, back{y - kYCollisionDistance},
              right{x + bbox.max_u}, front{y + kYCollisionDistance},
              top{z + bbox.height()}, bottom{z}, vx{vx}, vy{vy}, vz{vz},
              bbox{bbox}
        {
        }

        float left, back, right, front, top, bottom, vx, vy, vz;
        geometry::bbox<float> bbox;
    };

    /**
     * @enum type
     * @brief The type of physics that affects an entity.
     */
    enum class type : core::physics_t {
        NONE = 0x01,
        ACTOR = 0x02,
        STATIC = 0x04,
        SENSOR = 0x08,
        PROJECTILE = 0x10,
    };

    constexpr core::physics_t operator&(type a, type b)
    {
        return static_cast<core::physics_t>(a) &
                static_cast<core::physics_t>(b);
    }

    constexpr core::physics_t operator&(const core::physics_t a, type b)
    {
        return a & static_cast<core::physics_t>(b);
    }

    constexpr core::physics_t operator|(type a, type b)
    {
        return static_cast<core::physics_t>(a) |
                static_cast<core::physics_t>(b);
    }

    struct sweep_result {
        float time{1.0f};
        float normal_x{0.0f};
        float normal_y{0.0f};
        float normal_z{0.0f};
    };

    constexpr sweep_result sweep_aabb(const aabb& a, const aabb& b,
            const float vx, const float vy, const float vz)
    {
        sweep_result result;

        float entry_tx, exit_tx;
        if (std::abs(vx) < core::kEpsilon) {
            if (a.right < b.left || a.left > b.right) {
                return result;
            }
            entry_tx = -core::kInfinity;
            exit_tx = core::kInfinity;
        }
        else {
            const float in_x{vx > 0.0f ? b.left - a.right : b.right - a.left};
            const float out_x{vx > 0.0f ? b.right - a.left : b.left - a.right};

            entry_tx = in_x / vx;
            exit_tx = out_x / vx;
        }

        float entry_ty, exit_ty;
        if (std::abs(vy) < core::kEpsilon) {
            if (a.front < b.back || a.back > b.front) {
                return result;
            }
            entry_ty = -core::kInfinity;
            exit_ty = core::kInfinity;
        }
        else {
            const float in_y{vy > 0.0f ? b.back - a.front : b.front - a.back};
            const float out_y{vy > 0.0f ? b.front - a.back : b.back - a.front};

            entry_ty = in_y / vy;
            exit_ty = out_y / vy;
        }

        float entry_tz, exit_tz;
        if (std::abs(vz) < core::kEpsilon) {
            if (a.bottom > b.top || a.top < b.bottom) {
                return result;
            }
            entry_tz = -core::kInfinity;
            exit_tz = core::kInfinity;
        }
        else {
            const float in_z{vz > 0.0f ? b.bottom - a.top : b.top - a.bottom};
            const float out_z{vz > 0.0f ? b.top - a.bottom : b.bottom - a.top};

            entry_tz = in_z / vz;
            exit_tz = out_z / vz;
        }

        const float entry_t{std::max(entry_tx, std::max(entry_ty, entry_tz))};
        if (entry_t > std::min(exit_tx, std::min(exit_ty, exit_tz)) ||
                entry_t > 1.0f || entry_t < 0.0f ||
                (exit_tx < 0.0f && exit_ty < 0.0f && entry_tz < 0.0f)) {
            return result;
        }

        result.time = entry_t;
        if (entry_tx >= entry_ty && entry_tx >= entry_tz) {
            result.normal_x = vx > 0.0f ? -1.0f : 1.0f;
            result.normal_y = 0.0f;
            result.normal_z = 0.0f;
        }
        else if (entry_ty >= entry_tx && entry_ty >= entry_tz) {
            result.normal_x = 0.0f;
            result.normal_y = vy > 0.0f ? -1.0f : 1.0f;
            result.normal_z = 0.0f;
        }
        else {
            result.normal_x = 0.0f;
            result.normal_y = 0.0f;
            result.normal_z = vz > 0.0f ? 1.0f : -1.0f;
        }

        return result;
    }

    constexpr void sort_compute(scene_registry& registry) noexcept
    {
        std::ranges::sort(registry.physics_order.begin(),
                registry.physics_order.end(),
                [&](const core::index_t a, const core::index_t b) -> bool {
                    return registry.pos_x_ptr()[a] < registry.pos_x_ptr()[b];
                });
    }

    constexpr aabb from_registry(const scene_registry& registry,
            const core::index_t i, const sprites::metadata& metadata)
    {
        return aabb{registry.pos_x_ptr()[i], registry.pos_y_ptr()[i],
                registry.pos_z_ptr()[i], registry.vec_x_ptr()[i], registry.vec_y_ptr()[i],
                registry.vec_z_ptr()[i],
                static_cast<geometry::bbox<float>>(metadata.bbox)};
    }

    template<typename Iterator>
    sweep_result find_closest_hit(const aabb& box_a, const core::index_t idx_a,
            Iterator begin, Iterator end, const scene_registry& registry,
            const sprites::atlas& atlas, const float dt, const bool check_left)
    {
        sweep_result hit;

        const float vx{registry.vec_x_ptr()[idx_a] * dt};
        const float a_limit{check_left
                        ? std::min(box_a.left, box_a.left + vx)
                        : std::max(box_a.right, box_a.right + vx)};

        for (Iterator it{begin}; it != end; ++it) {
            const core::index_t idx_b{*it};
            const sprites::metadata& sprite_b{
                    atlas[registry.indices[idx_b]].meta};

            if (sprite_b.physics_type & type::NONE) {
                continue;
            }

            const aabb box_b{from_registry(registry, idx_b, sprite_b)};
            if (check_left ? a_limit > box_b.right : a_limit < box_b.left)
                break;

            const sweep_result result{sweep_aabb(box_a, box_b,
                    (registry.vec_x_ptr()[idx_a] - registry.vec_x_ptr()[idx_b]) * dt,
                    (registry.vec_y_ptr()[idx_a] - registry.vec_y_ptr()[idx_b]) * dt,
                    (registry.vec_z_ptr()[idx_a] - registry.vec_z_ptr()[idx_b]) * dt)};

            if (result.time < hit.time) {
                hit = result;
            }
        }

        return hit;
    }

    constexpr void resolve_entity_collisions(const sprites::atlas& atlas,
            scene_registry& registry, const float dt)
    {
        sort_compute(registry);

        for (core::index_t i{0u}; i < registry.count(); ++i) {
            const core::index_t idx_a{registry.physics_order[i]};
            const sprites::metadata& sprite_a{
                    atlas[registry.indices[idx_a]].meta};

            if (!(sprite_a.physics_type & type::ACTOR)) {
                continue;
            }

            if (std::abs(registry.vec_x_ptr()[idx_a]) < core::kEpsilon &&
                    std::abs(registry.vec_y_ptr()[idx_a]) < core::kEpsilon) {
                registry.new_x_ptr()[idx_a] = registry.pos_x_ptr()[idx_a];
                registry.new_y_ptr()[idx_a] = registry.pos_y_ptr()[idx_a];
                registry.new_z_ptr()[idx_a] = registry.pos_z_ptr()[idx_a];
                continue;
            }

            if (std::abs(registry.vec_y_ptr()[idx_a]) > core::kEpsilon) {
                registry.needs_sort = true;
            }

            const aabb a{from_registry(registry, idx_a, sprite_a)};

            // sweep_result hit{find_closest_hit(a, idx_a,
            //         registry.physics_order.begin() + i,
            //         registry.physics_order.end(), registry, atlas, dt,
            //         true)};

            const float vx{registry.vec_x_ptr()[idx_a] * dt};
            const float a_min_x{std::min(a.left, a.left + vx)};
            const float a_max_x{std::max(a.right, a.right + vx)};

            sweep_result hit;

            for (core::index_t j{i + 1u}; j < registry.count(); ++j) {
                const core::index_t index_b{registry.physics_order[j]};
                const sprites::metadata& sprite_b{
                        atlas[registry.indices[index_b]].meta};

                if (sprite_b.physics_type & type::NONE) {
                    continue;
                }

                const aabb b{from_registry(registry, index_b, sprite_b)};
                if (a_max_x < b.left)
                    break;

                const sweep_result result{sweep_aabb(a, b,
                        (registry.vec_x_ptr()[idx_a] - registry.vec_x_ptr()[index_b]) *
                                dt,
                        (registry.vec_y_ptr()[idx_a] - registry.vec_y_ptr()[index_b]) *
                                dt,
                        (registry.vec_z_ptr()[idx_a] - registry.vec_z_ptr()[index_b]) *
                                dt)};

                if (result.time < hit.time) {
                    hit = result;
                }
            }

            for (int32_t j{static_cast<int32_t>(i) - 1}; j >= 0; --j) {
                const core::index_t idx_b{registry.physics_order[j]};
                const sprites::metadata& sprite_b{
                        atlas[registry.indices[idx_b]].meta};

                if (sprite_b.physics_type & type::NONE) {
                    continue;
                }

                const aabb b{from_registry(registry, idx_b, sprite_b)};
                if (a_min_x > b.right)
                    break;

                const sweep_result result{sweep_aabb(a, b,
                        (registry.vec_x_ptr()[idx_a] - registry.vec_x_ptr()[idx_b]) *
                                dt,
                        (registry.vec_y_ptr()[idx_a] - registry.vec_y_ptr()[idx_b]) *
                                dt,
                        (registry.vec_z_ptr()[idx_a] - registry.vec_z_ptr()[idx_b]) *
                                dt)};

                if (result.time < hit.time) {
                    hit = result;
                }
            }

            const float padded_t{std::max(0.0f, hit.time - 0.1f)};

            registry.new_x_ptr()[idx_a] = registry.pos_x_ptr()[idx_a] +
                    registry.vec_x_ptr()[idx_a] * dt * padded_t;
            registry.new_y_ptr()[idx_a] = registry.pos_y_ptr()[idx_a] +
                    registry.vec_y_ptr()[idx_a] * dt * padded_t;
            registry.new_z_ptr()[idx_a] = registry.pos_z_ptr()[idx_a] +
                    registry.vec_z_ptr()[idx_a] * dt * padded_t;

            if (hit.time < 1.0f) {
                const float remain_t{1.0f - hit.time};

                const float dot{registry.vec_x_ptr()[idx_a] * hit.normal_x +
                        registry.vec_y_ptr()[idx_a] * hit.normal_y +
                        registry.vec_z_ptr()[idx_a] * hit.normal_z};

                const float slide_x{
                        registry.vec_x_ptr()[idx_a] - dot * hit.normal_x};
                const float slide_y{
                        registry.vec_y_ptr()[idx_a] - dot * hit.normal_y};
                const float slide_z{
                        registry.vec_z_ptr()[idx_a] - dot * hit.normal_z};

                registry.new_x_ptr()[idx_a] += slide_x * dt * remain_t;
                registry.new_y_ptr()[idx_a] += slide_y * dt * remain_t;
                registry.new_z_ptr()[idx_a] += slide_z * dt * remain_t;

                registry.vec_x_ptr()[idx_a] = slide_x;
                registry.vec_y_ptr()[idx_a] = slide_y;
                registry.vec_z_ptr()[idx_a] = slide_z;
            }
        }
    }

} // namespace sc::physics
