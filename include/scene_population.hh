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

        /// TODO: Make custom allocator
        std::vector<float> x, y, z, dx, dy;
        std::vector<sprites::atlas_index> indices;
        std::vector<core::index_t> draw_order;

        explicit scene_population(
                std::size_t reserve_count = core::kAlignment) noexcept;

        scene_population(const scene_population&) = delete;
        scene_population(scene_population&&) = delete;
        scene_population& operator=(const scene_population&) = delete;
        scene_population& operator=(scene_population&&) = delete;

        [[nodiscard]] constexpr std::size_t size() const noexcept;

        /**
         * @brief Reserve space in the registry.
         * @param n The number of entries to reserve.
         */
        void reserve(std::size_t n) noexcept;

        /**
         * @brief Add a new entity to the layout.
         * @param start_x The starting horizontal position.
         * @param start_y The starting vertical position.
         * @param start_z The starting aerial position.
         * @param i The entity's ID.
         */
        void spawn(float start_x, float start_y, float start_z,
                sprites::atlas_index i) noexcept;

        void resolve_collision(const sprites::atlas& bank,
                const sprites::sprite& a, core::index_t ai, float& ax,
                float& ay, float& az) noexcept;

        /**
         * @brief Update the current layout.
         * @param bank The sprites::sprite bank.
         * @param dt The delta time.
         * @param display_width The display's width.
         * @param display_height The display's height.
         */
        void update(const sprites::atlas& bank, float dt, float display_width,
                float display_height) noexcept;

    private:
        bool needs_sort_{false};
    };

    inline scene_population::scene_population(
            const std::size_t reserve_count) noexcept
    {
        this->reserve(reserve_count < core::kAlignment ? core::kAlignment
                                                       : reserve_count);
    }

    [[nodiscard]] constexpr std::size_t scene_population::size() const noexcept
    {
        return x.size();
    }

    inline void scene_population::reserve(const std::size_t n) noexcept
    {
        const std::size_t aligned{
                n + core::kAlignment - 1 & ~(core::kAlignment - 1)};

        if (aligned > x.capacity()) {
            x.reserve(aligned);
            y.reserve(aligned);
            z.reserve(aligned);
            dx.reserve(aligned);
            dy.reserve(aligned);
            indices.reserve(aligned);
            draw_order.reserve(aligned);
        }
    }

    inline void scene_population::spawn(const float start_x,
            const float start_y, const float start_z,
            const sprites::atlas_index i) noexcept
    {
        if (x.capacity() == x.size()) [[unlikely]]
            this->reserve(x.size() * 2);

        x.push_back(start_x);
        y.push_back(start_y);
        z.push_back(start_z);
        dx.push_back(0.0f);
        dy.push_back(0.0f);
        indices.push_back(i);
        draw_order.push_back(static_cast<std::uint32_t>(x.size() - 1));

        needs_sort_ = true;
    }

    /**
     * @brief Check if 2 entities overlap.
     * @param a The 1st entity.
     * @param ax The 1st entity's x-coordinate.
     * @param ay The 2st entity's y-coordinate.
     * @param az The 2st entity's z-coordinate.
     * @param b The 2nd entity.
     * @param bx The 2nd entity's x-coordinate.
     * @param by The 2nd entity's y-coordinate.
     * @param bz The 2nd entity's z-coordinate.
     * @return
     */
    inline bool has_collision(const sprites::sprite& a, const float ax,
            const float ay, const float az, const sprites::sprite& b,
            const float bx, const float by, const float bz) noexcept
    {
        const bool overlap_x1{ax + static_cast<float>(a.left) <
                bx + static_cast<float>(b.right)};
        const bool overlap_x2{ax + static_cast<float>(a.right) >
                bx + static_cast<float>(b.left)};

        const float a_bottom{ay + static_cast<float>(a.bottom)};
        const float b_bottom{by + static_cast<float>(b.bottom)};
        const bool overlap_y{std::abs(a_bottom - b_bottom) < 5.0f};

        return overlap_x1 && overlap_x2 && overlap_y;
    }

    inline void scene_population::resolve_collision(const sprites::atlas& bank,
            const sprites::sprite& a, const core::index_t ai, float& ax,
            float& ay, float& az) noexcept
    {
        for (core::index_t i{0}; i < indices.size(); ++i) {
            if (ai == i)
                continue;

            if (const sprites::sprite& b{bank[indices[i]]};
                    has_collision(a, ax, ay, az, b, x[i], y[i], z[i])) {
                ax = x[ai];
                ay = y[ai];
                az = z[ai];
                dx[ai] = 0.0f;
                dy[ai] = 0.0f;
                return;
            }
        }
    }

    inline void scene_population::update(const sprites::atlas& bank,
            const float dt, const float display_width,
            const float display_height) noexcept
    {
        for (std::size_t i{0}; i < indices.size(); ++i) {
            const sprites::atlas_index id{indices[i]};
            const auto& sprite{bank[id]};

            float& rx{x[i]};
            float& ry{y[i]};
            float& rz{z[i]};

            float& rdx{dx[i]};
            float& rdy{dy[i]};

            if (rdx == 0 && rdy == 0)
                continue;

            float next_x{rx + rdx * dt};
            float next_y{ry + rdy * dt};

            if (next_x + static_cast<float>(sprite.left) < 0.0f) {
                next_x = -static_cast<float>(sprite.left);
                rdx = 0.0f;
            }
            else if (next_x + static_cast<float>(sprite.right) >
                    display_width) {
                next_x = display_width - static_cast<float>(sprite.right);
                rdx = 0.0f;
            }

            if (next_y + static_cast<float>(sprite.top) < 0) {
                next_y = -static_cast<float>(sprite.top);
                rdy = 0.0f;
            }
            else if (next_y + static_cast<float>(sprite.bottom) >
                    display_height) {
                next_y = display_height - static_cast<float>(sprite.bottom);
                rdy = 0.0f;
            }

            resolve_collision(bank, sprite, i, next_x, next_y, rz);

            rx = next_x;
            ry = next_y;

            if (std::abs(dy[i]) > std::numeric_limits<float>::epsilon())
                needs_sort_ = true;
        }

        if (needs_sort_) {
            std::ranges::sort(draw_order.begin(), draw_order.end(),
                    [&](const uint32_t a, const uint32_t b) {
                        return y[a] < y[b];
                    });
            needs_sort_ = false;
        }
    }

} // namespace sc
