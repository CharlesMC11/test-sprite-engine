/**
 * @file scene_population.hh
 * @brief
 */
#pragma once

#include <algorithm>
#include <cstddef>
#include <numeric>
#include <vector>

#include "atlas.hh"
#include "atlas_index.hh"
#include "core.hh"
#include "sprite.hh"

namespace sc {

    /**
     * @struct scene_population
     * @brief
     */
    struct scene_population final {
        explicit constexpr scene_population(
                std::size_t reserve_count = core::kAlignment) noexcept;

        scene_population(const scene_population&) = delete;
        scene_population(scene_population&&) = default;

        ~scene_population() = default;

        scene_population& operator=(const scene_population&) = delete;
        scene_population& operator=(scene_population&&) = default;

        [[nodiscard]] constexpr std::size_t size() const noexcept;

        /**
         * @brief Reserve space in the registry.
         * @param n The number of entries to reserve.
         */
        constexpr void reserve(std::size_t n) noexcept;

        /**
         * @brief Add a new entity to the layout.
         * @param start_x The starting horizontal position.
         * @param start_y The starting vertical position.
         * @param start_z The starting aerial position.
         * @param i The entity's ID.
         */
        constexpr void spawn(float start_x, float start_y, float start_z,
                sprites::atlas_index i) noexcept;

        /**
         * @brief Update the current layout.
         * @param dt The delta time.
         */
        constexpr void update(float dt) noexcept;

        constexpr void sort_draw() noexcept;

        /// TODO: Make custom allocator?
        std::vector<float> x, y, z, dx, dy, dz, next_x, next_y, next_z;
        std::vector<sprites::atlas_index> indices;
        std::vector<core::index_t> draw_order;

        bool needs_sort{false};
    };

    constexpr scene_population::scene_population(
            const std::size_t reserve_count) noexcept
    {
        reserve(reserve_count < core::kAlignment ? core::kAlignment
                                                 : reserve_count);
    }

    [[nodiscard]] constexpr std::size_t scene_population::size() const noexcept
    {
        return x.size();
    }

    constexpr void scene_population::reserve(const std::size_t n) noexcept
    {
        const std::size_t aligned{
                n + core::kAlignment - 1 & ~(core::kAlignment - 1)};

        if (aligned > x.capacity()) {
            x.reserve(aligned);
            y.reserve(aligned);
            z.reserve(aligned);
            dx.reserve(aligned);
            dy.reserve(aligned);
            dz.reserve(aligned);
            next_x.reserve(aligned);
            next_y.reserve(aligned);
            next_z.reserve(aligned);
            indices.reserve(aligned);
            draw_order.reserve(aligned);
        }
    }

    constexpr void scene_population::spawn(const float start_x,
            const float start_y, const float start_z,
            const sprites::atlas_index i) noexcept
    {
        if (x.capacity() == x.size()) [[unlikely]]
            reserve(x.size() * 2);

        x.push_back(start_x);
        y.push_back(start_y);
        z.push_back(start_z);
        dx.push_back(0.0f);
        dy.push_back(0.0f);
        dz.push_back(0.0f);
        next_x.push_back(start_x);
        next_y.push_back(start_y);
        next_z.push_back(start_z);
        indices.push_back(i);
        draw_order.push_back(static_cast<core::index_t>(x.size() - 1));

        needs_sort = true;
    }

    constexpr void scene_population::update(const float dt) noexcept
    {
        for (core::index_t i{0}; i < x.size(); ++i) {
            next_x[i] = x[i] + dx[i] * dt;
            next_y[i] = y[i] + dy[i] * dt;
            next_z[i] = z[i] + dz[i] * dt;
        }
    }

    constexpr void scene_population::sort_draw() noexcept
    {
        if (needs_sort) {
            std::ranges::sort(draw_order.begin(), draw_order.end(),
                    [&](const core::index_t a, const core::index_t b) noexcept
                            -> bool { return y[a] < y[b]; });

            needs_sort = false;
        }
    }

} // namespace sc
