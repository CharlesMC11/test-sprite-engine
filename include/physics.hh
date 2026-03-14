/**
 * @file physics.hh
 * @brief
 */
#pragma once

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
              height{bbox.bottom - bbox.top}
        {
        }

        float left, right, top, bottom, altitude, height;
    };

    constexpr void resolve_entity_collisions(
            const sprites::atlas& atlas, scene_population& registry)
    {
        for (core::index_t index_a{0}; index_a < registry.size(); ++index_a) {
            const sprites::sprite& sprite_a{atlas[registry.indices[index_a]]};

            if (sprite_a.physics &
                    (sprites::physics_type::NONE |
                            sprites::physics_type::STATIC |
                            sprites::physics_type::SENSOR))
                continue;

            const entity a{registry.next_x[index_a], registry.next_y[index_a],
                    registry.next_z[index_a],
                    {static_cast<float>(sprite_a.left),
                            static_cast<float>(sprite_a.top),
                            static_cast<float>(sprite_a.right),
                            static_cast<float>(sprite_a.bottom)}};

            if (registry.dy[index_a] > 0.0f)
                registry.needs_sort = true;

            for (core::index_t index_b{0}; index_b < registry.size();
                    ++index_b) {
                if (index_a == index_b)
                    continue;

                const sprites::sprite& sprite_b{
                        atlas[registry.indices[index_b]]};
                if (sprite_b.physics & sprites::physics_type::NONE)
                    continue;

                const entity b{registry.next_x[index_b],
                        registry.next_y[index_b], registry.next_z[index_b],
                        {static_cast<float>(sprite_b.left),
                                static_cast<float>(sprite_b.top),
                                static_cast<float>(sprite_b.right),
                                static_cast<float>(sprite_b.bottom)}};

                const bool collides_left{a.left < b.right};
                const bool collides_right{a.right > b.left};

                const bool collides_top{
                        a.bottom < b.bottom + kYCollisionDistance};
                const bool collides_bottom{
                        a.bottom > b.bottom - kYCollisionDistance};

                const bool collides_altitude{
                        a.altitude < b.altitude + b.height ||
                        a.altitude + a.height > b.altitude};

                if (collides_left && collides_right && collides_top &&
                        collides_bottom && collides_altitude) {
                    registry.next_x[index_a] = registry.x[index_a];
                    registry.next_y[index_a] = registry.y[index_a];
                    registry.next_z[index_a] = registry.z[index_a];
                    registry.dx[index_a] = 0.0f;
                    registry.dy[index_a] = 0.0f;
                    registry.dz[index_a] = 0.0f;
                }

                // if (collides_left) {
                //     registry.next_x[index_a] = entity_b.x +
                //     entity_b.bbox.right; registry.dx[index_a] = 0.0f;
                // }
                // else if (collides_right) {
                //     registry.next_x[index_a] = entity_b.x +
                //     entity_b.bbox.left; registry.dx[index_a] = 0.0f;
                // }

                // if (collides_bottom) {
                //     registry.next_y[a] = entity_b.bbox.bottom + 0.5f;
                //     registry.dy[a] = 0.0f;
                // }
                // else if (collides_top) {
                //     registry.next_y[a] = entity_b.bbox.bottom - 0.5f;
                //     registry.dy[a] = 0.0f;
                // }
            }
        }

        registry.x = registry.next_x;
        registry.y = registry.next_y;
        registry.z = registry.next_z;

        registry.sort_draw();
    }

} // namespace sc::physics
