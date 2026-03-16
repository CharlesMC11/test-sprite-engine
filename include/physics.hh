/**
 * @file physics.hh
 * @brief
 */
#pragma once

#include "Metal/MTLHeaderBridge.hpp"
#include "atlas.hh"
#include "bbox.hh"
#include "core.hh"
#include "scene_registry.hh"

namespace sc::physics {

    static constexpr float kGravity{9.8f};
    static constexpr float kFixedTimestep{1.0f / 120.0f};
    static constexpr float kMaxVelocity{500.0f};

    static constexpr float kYCollisionDistance{8.0f};

    struct aabb {
        explicit aabb(const float x, const float y, const float z,
                const float vx, const float vy, const float vz,
                const geometry::bbox<float>& bbox) noexcept
            : left{x + bbox.left}, back{y - kYCollisionDistance},
              right{x + bbox.right}, front{y + kYCollisionDistance},
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
        float time{1.0f}, normal_x{0.0f}, normal_y{0.0f};
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
        if (entry_tx > entry_ty) {
            result.normal_x = vx > 0.0f ? -1.0f : 1.0f;
            result.normal_y = 0.0f;
        }
        else {
            result.normal_x = 0.0f;
            result.normal_y = vy > 0.0f ? -1.0f : 1.0f;
        }

        return result;
    }

    constexpr void resolve_entity_collisions(const sprites::atlas& atlas,
            scene_registry& registry, const float dt)
    {
        for (core::index_t index_a{0u}; index_a < registry.size(); ++index_a) {
            const sprites::metadata& sprite_a{
                    atlas[registry.indices[index_a]].metadata};

            if (!(sprite_a.physics & type::ACTOR)) {
                continue;
            }

            if (std::abs(registry.vx[index_a]) < core::kEpsilon &&
                    std::abs(registry.vy[index_a]) < core::kEpsilon) {
                registry.next_x[index_a] = registry.x[index_a];
                registry.next_y[index_a] = registry.y[index_a];
                registry.next_z[index_a] = registry.z[index_a];
                continue;
            }

            if (std::abs(registry.vy[index_a]) > core::kEpsilon) {
                registry.needs_sort = true;
            }

            const geometry::bbox bbox_a{static_cast<float>(sprite_a.bbox.left),
                    static_cast<float>(sprite_a.bbox.top),
                    static_cast<float>(sprite_a.bbox.right),
                    static_cast<float>(sprite_a.bbox.bottom)};
            const aabb a{registry.x[index_a], registry.y[index_a],
                    registry.z[index_a], registry.vx[index_a],
                    registry.vy[index_a], registry.vz[index_a], bbox_a};

            sweep_result hit;

            for (core::index_t index_b{0u}; index_b < registry.size();
                    ++index_b) {

                if (index_a == index_b) {
                    continue;
                }

                const sprites::metadata& sprite_b{
                        atlas[registry.indices[index_b]].metadata};
                if (sprite_b.physics & type::NONE) {
                    continue;
                }

                const geometry::bbox bbox_b{
                        static_cast<float>(sprite_b.bbox.left),
                        static_cast<float>(sprite_b.bbox.top),
                        static_cast<float>(sprite_b.bbox.right),
                        static_cast<float>(sprite_b.bbox.bottom)};
                const aabb b{registry.x[index_b], registry.y[index_b],
                        registry.z[index_b], registry.vx[index_b],
                        registry.vy[index_b], registry.vz[index_b], bbox_b};

                const sweep_result result{sweep_aabb(a, b,
                        (registry.vx[index_a] - registry.vx[index_b]) * dt,
                        (registry.vy[index_a] - registry.vy[index_b]) * dt,
                        (registry.vz[index_a] - registry.vz[index_b]) * dt)};

                if (result.time < hit.time) {
                    hit = result;
                }
            }

            const float padded_t{std::max(0.0f, hit.time - core::kEpsilon)};

            registry.next_x[index_a] =
                    registry.x[index_a] + registry.vx[index_a] * dt * padded_t;
            registry.next_y[index_a] =
                    registry.y[index_a] + registry.vy[index_a] * dt * padded_t;

            if (hit.time < 1.0f) {
                const float remain_t{1.0f - hit.time};

                const float dot{registry.vx[index_a] * hit.normal_x +
                        registry.vy[index_a] * hit.normal_y};

                const float slide_x{registry.vx[index_a] - dot * hit.normal_x};
                const float slide_y{registry.vy[index_a] - dot * hit.normal_y};

                registry.next_x[index_a] += slide_x * dt * remain_t;
                registry.next_y[index_a] += slide_y * dt * remain_t;

                registry.vx[index_a] = slide_x;
                registry.vz[index_a] = slide_y;
            }
        }
    }

} // namespace sc::physics
