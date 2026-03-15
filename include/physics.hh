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
                const geometry::bbox<float>& bbox) noexcept
            : left{x + bbox.left}, right{x + bbox.right}, top{y + bbox.top},
              bottom{y + bbox.bottom}, altitude{z + bbox.bottom}, bbox{bbox}
        {
        }

        float left, right, top, bottom, altitude;
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

    constexpr void resolve_entity_collision(scene_registry& registry,
            const aabb& a, const geometry::bbox<float>& bbox_a,
            const core::index_t index_a, const aabb& b)
    {
        bool collides_left{a.left < b.right};
        bool collides_right{a.right > b.left};

        const bool collides_top{a.bottom < b.bottom + kYCollisionDistance};
        const bool collides_bottom{a.bottom > b.bottom - kYCollisionDistance};

        const bool collides_altitude{
                a.altitude < b.altitude + b.bbox.height() &&
                a.altitude + a.bbox.height() > b.altitude};

        if (collides_left && collides_right && collides_top &&
                collides_bottom && collides_altitude) {

            const float prev_x_l{registry.x[index_a] + bbox_a.left};
            const float prev_x_r{registry.x[index_a] + bbox_a.right};
            const float prev_y{registry.y[index_a] + bbox_a.bottom};

            collides_left = prev_x_l < b.right;
            collides_right = prev_x_r > b.left;

            const bool collides_vertical{
                    std::abs(prev_y - b.bottom) < kYCollisionDistance};

            if (collides_left || collides_right) {
                registry.next_x[index_a] = registry.x[index_a];
                registry.dx[index_a] = 0.0f;
            }

            if (collides_vertical) {
                registry.next_y[index_a] = registry.y[index_a];
                registry.dy[index_a] = 0.0f;
            }
        }
    }

    constexpr void resolve_entity_collisions(
            const sprites::atlas& atlas, scene_registry& registry)
    {
        for (core::index_t index_a{0u}; index_a < 1; ++index_a) {
            const sprites::sprite& sprite_a{atlas[registry.indices[index_a]]};

            if (!(sprite_a.physics & type::ACTOR)) {
                continue;
            }

            const geometry::bbox bbox_a{static_cast<float>(sprite_a.bbox.left),
                    static_cast<float>(sprite_a.bbox.top),
                    static_cast<float>(sprite_a.bbox.right),
                    static_cast<float>(sprite_a.bbox.bottom)};
            const aabb a{registry.next_x[index_a], registry.next_y[index_a],
                    registry.next_z[index_a], bbox_a};

            if (registry.dy[index_a] > 0.0f) {
                registry.needs_sort = true;
            }

            for (core::index_t index_b{0u}; index_b < registry.size();
                    ++index_b) {

                if (index_a == index_b) {
                    continue;
                }

                const sprites::sprite& sprite_b{
                        atlas[registry.indices[index_b]]};
                if (sprite_b.physics & type::NONE) {
                    continue;
                }

                const geometry::bbox bbox_b{
                        static_cast<float>(sprite_b.bbox.left),
                        static_cast<float>(sprite_b.bbox.top),
                        static_cast<float>(sprite_b.bbox.right),
                        static_cast<float>(sprite_b.bbox.bottom)};
                const aabb b{registry.next_x[index_b], registry.next_y[index_b],
                        registry.next_z[index_b], bbox_b};

                resolve_entity_collision(registry, a, bbox_a, index_a, b);
            }
        }
    }

} // namespace sc::physics
