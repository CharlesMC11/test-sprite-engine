/**
 * @file transform_registry.h
 * @brief
 */
#pragma once

#include <vector>

#include "atlas_index.hpp"

namespace sc {

    struct transform_registry {

        alignas(16) std::vector<float> x;
        alignas(16) std::vector<float> y;

        alignas(16) std::vector<float> vx;
        alignas(16) std::vector<float> vy;

        alignas(16) std::vector<atlas_index> sprite_ids;

        [[nodiscard]] constexpr std::size_t size() const noexcept;

        void update(float dt) noexcept;

        void add_entity(float start_x, float start_y, atlas_index id) noexcept;
    };

    [[nodiscard]] constexpr std::size_t
    transform_registry::size() const noexcept
    {
        return x.size();
    }

    inline void transform_registry::update(const float dt) noexcept
    {
        /// TODO: Use NEON / assembly
        for (std::size_t i{0}; i < x.size(); ++i) {
            x[i] += vx[i] * dt;
            y[i] += vy[i] * dt;
        }
    }

    inline void transform_registry::add_entity(const float start_x,
            const float start_y, const atlas_index id) noexcept
    {
        /// TODO: Maybe use NEON / assembly
        x.push_back(start_x);
        y.push_back(start_y);
        vx.push_back(0.0f);
        vy.push_back(0.0f);
        sprite_ids.push_back(id);
    }

} // namespace sc
