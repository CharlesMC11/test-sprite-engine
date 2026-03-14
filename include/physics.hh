/**
 * @file physics.hh
 * @brief
 */
#pragma once

#include "Metal/MTLHeaderBridge.hpp"
#include "atlas.hh"
#include "core.hh"
#include "scene_population.hh"

namespace sc::physics {

    struct bbox {
        float left, top, right, bottom;
    };

    struct entity {
        explicit entity(const float x, const float y, const float z,
                const bbox& bbox) noexcept
            : left{x + bbox.left}, right{x + bbox.right}, top{y + bbox.top},
              bottom{y + bbox.bottom}, altitude{z + bbox.bottom},
              height{bbox.bottom - bbox.top}, bbox{bbox}
        {
        }

        float left, right, top, bottom, altitude, height;
        bbox bbox;
    };

    constexpr void resolve_entity_collision(scene_population& registry,
            const entity& a, const bbox& bbox_a, const core::index_t index_a,
            const entity& b)
    {
        bool collides_left{a.left < b.right};
        bool collides_right{a.right > b.left};

        const bool collides_top{a.bottom < b.bottom + kYCollisionDistance};
        const bool collides_bottom{a.bottom > b.bottom - kYCollisionDistance};

        const bool collides_altitude{a.altitude < b.altitude + b.height &&
                a.altitude + a.height > b.altitude};

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
            const sprites::atlas& atlas, scene_population& registry)
    {
        for (core::index_t index_a{0u}; index_a < 1; ++index_a) {
            const sprites::sprite& sprite_a{atlas[registry.indices[index_a]]};

            if (!(sprite_a.physics & sprites::physics_type::ACTOR)) {
                continue;
            }

            const bbox bbox_a{static_cast<float>(sprite_a.left),
                    static_cast<float>(sprite_a.top),
                    static_cast<float>(sprite_a.right),
                    static_cast<float>(sprite_a.bottom)};
            const entity a{registry.next_x[index_a], registry.next_y[index_a],
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
                if (sprite_b.physics & sprites::physics_type::NONE) {
                    continue;
                }

                const bbox bbox_b{static_cast<float>(sprite_b.left),
                        static_cast<float>(sprite_b.top),
                        static_cast<float>(sprite_b.right),
                        static_cast<float>(sprite_b.bottom)};
                const entity b{registry.next_x[index_b],
                        registry.next_y[index_b], registry.next_z[index_b],
                        bbox_b};

                resolve_entity_collision(registry, a, bbox_a, index_a, b);
            }
        }
    }

} // namespace sc::physics
