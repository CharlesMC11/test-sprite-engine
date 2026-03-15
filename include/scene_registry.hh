/**
 * @file scene_population.hh
 * @brief
 */
#pragma once

#include <arm_neon.h>

#include <algorithm>
#include <cstddef>
#include <vector>

#include "atlas_index.hh"
#include "core.hh"

namespace sc {

    /**
     * @struct scene_registry
     * @brief
     */
    struct scene_registry final {
        explicit constexpr scene_registry(
                std::size_t reserve_count = core::kAlignment) noexcept;

        scene_registry(const scene_registry&) = delete;
        scene_registry(scene_registry&&) = default;

        ~scene_registry() = default;

        scene_registry& operator=(const scene_registry&) = delete;
        scene_registry& operator=(scene_registry&&) = default;

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

        constexpr void commit() noexcept;

        constexpr void sort_draw() noexcept;

        /// TODO: Make custom allocator?
        std::vector<float> x, y, z, dx, dy, dz, next_x, next_y, next_z;
        std::vector<sprites::atlas_index> indices;
        std::vector<core::index_t> draw_order;

        bool needs_sort{false};
    };

    constexpr scene_registry::scene_registry(
            const std::size_t reserve_count) noexcept
    {
        reserve(reserve_count < core::kAlignment ? core::kAlignment
                                                 : reserve_count);
    }

    [[nodiscard]] constexpr std::size_t scene_registry::size() const noexcept
    {
        return x.size();
    }

    constexpr void scene_registry::reserve(const std::size_t n) noexcept
    {
        const std::size_t aligned{
                n + core::kAlignment - 1u & ~(core::kAlignment - 1u)};

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

    constexpr void scene_registry::spawn(const float start_x,
            const float start_y, const float start_z,
            const sprites::atlas_index i) noexcept
    {
        if (x.capacity() == x.size()) [[unlikely]]
            reserve(x.size() * 2u);

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
        draw_order.push_back(static_cast<core::index_t>(x.size() - 1u));

        needs_sort = true;
    }

    constexpr void scene_registry::update(const float dt) noexcept
    {
        const auto n{static_cast<core::index_t>(x.size())};
        const core::index_t vectorized_lim{n - n % 4u};

        if (vectorized_lim > 0u) {
            const float32x4_t v_dt{vdupq_n_f32(dt)};
            for (core::index_t i{0u}; i < vectorized_lim; i += 4u) {
                const float32x4_t v_x{vld1q_f32(&x[i])};
                const float32x4_t v_y{vld1q_f32(&y[i])};
                const float32x4_t v_z{vld1q_f32(&z[i])};

                const float32x4_t v_dx{vld1q_f32(&dx[i])};
                const float32x4_t v_dy{vld1q_f32(&dy[i])};
                const float32x4_t v_dz{vld1q_f32(&dz[i])};

                const float32x4_t v_next_x{vfmaq_f32(v_x, v_dx, v_dt)};
                const float32x4_t v_next_y{vfmaq_f32(v_y, v_dy, v_dt)};
                const float32x4_t v_next_z{vfmaq_f32(v_z, v_dz, v_dt)};

                vst1q_f32(&next_x[i], v_next_x);
                vst1q_f32(&next_y[i], v_next_y);
                vst1q_f32(&next_z[i], v_next_z);
            }
        }

        for (core::index_t i{vectorized_lim}; i < n; ++i) {
            next_x[i] = x[i] + dx[i] * dt;
            next_y[i] = y[i] + dy[i] * dt;
            next_z[i] = z[i] + dz[i] * dt;
        }
    }

    constexpr void scene_registry::commit() noexcept
    {
        x = next_x;
        y = next_y;
        z = next_z;
    }

    constexpr void scene_registry::sort_draw() noexcept
    {
        if (needs_sort) {
            std::ranges::sort(draw_order.begin(), draw_order.end(),
                    [&](const core::index_t a, const core::index_t b) noexcept
                            -> bool { return y[a] < y[b]; });

            needs_sort = false;
        }
    }

} // namespace sc
