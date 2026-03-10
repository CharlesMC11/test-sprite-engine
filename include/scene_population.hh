/**
 * @file entity_layout.hpp
 * @brief
 */
#pragma once

#include <arm_neon.h>

#include <algorithm>
#include <numeric>
#include <vector>

#include "definitions.hpp"
#include "entity_id.hpp"
#include "sprite.hpp"
#include "sprite_bank.hpp"

namespace sc {

    /**
     * @struct entity_layout
     * @brief
     */
    struct entity_layout final {

        /// TODO: Make custom allocator
        std::vector<float> x, y, z, dx, dy;
        std::vector<entity_id> entity_ids;
        std::vector<sys::index_t> draw_order;

        explicit entity_layout(
                std::size_t reserve_count = sys::ALIGNMENT) noexcept;

        entity_layout(const entity_layout&) = delete;
        entity_layout(entity_layout&&) = delete;
        entity_layout& operator=(const entity_layout&) = delete;
        entity_layout& operator=(entity_layout&&) = delete;

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
         * @param id The entity's ID.
         */
        void spawn(float start_x, float start_y, float start_z,
                entity_id id) noexcept;

        void resolve_collision(const sprite_bank& bank, const sprite& a,
                sys::index_t ai, float& ax, float& ay, float& az) noexcept;

        /**
         * @brief Update the current layout.
         * @param bank The sprite bank.
         * @param dt The delta time.
         * @param display_width The display's width.
         * @param display_height The display's height.
         */
        void update(const sprite_bank& bank, float dt, float display_width,
                float display_height) noexcept;

    private:
        bool needs_sort_{false};
    };

    inline entity_layout::entity_layout(
            const std::size_t reserve_count) noexcept
    {
        this->reserve(reserve_count < sys::ALIGNMENT ? sys::ALIGNMENT
                                                     : reserve_count);
    }

    [[nodiscard]] constexpr std::size_t entity_layout::size() const noexcept
    {
        return x.size();
    }

    inline void entity_layout::reserve(const std::size_t n) noexcept
    {
        const std::size_t aligned{
                (n + sys::ALIGNMENT - 1) & ~(sys::ALIGNMENT - 1)};

        if (aligned > x.capacity()) {
            x.reserve(aligned);
            y.reserve(aligned);
            z.reserve(aligned);
            dx.reserve(aligned);
            dy.reserve(aligned);
            entity_ids.reserve(aligned);
            draw_order.reserve(aligned);
        }
    }

    inline void entity_layout::spawn(const float start_x, const float start_y,
            const float start_z, const entity_id id) noexcept
    {
        if (x.capacity() == x.size()) [[unlikely]]
            this->reserve(x.size() * 2);

        x.push_back(start_x);
        y.push_back(start_y);
        z.push_back(start_z);
        dx.push_back(0.0f);
        dy.push_back(0.0f);
        entity_ids.push_back(id);
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
    inline bool has_collision(const sprite& a, const float ax, const float ay,
            const float az, const sprite& b, const float bx, const float by,
            const float bz) noexcept
    {
        const bool overlap_x1{ax + static_cast<float>(a.hb_left) <
                bx + static_cast<float>(b.hb_right)};
        const bool overlap_x2{ax + static_cast<float>(a.hb_right) >
                bx + static_cast<float>(b.hb_left)};

        const float a_bottom{ay + static_cast<float>(a.hb_bottom)};
        const float b_bottom{by + static_cast<float>(b.hb_bottom)};
        const bool overlap_y{std::abs(a_bottom - b_bottom) < 5.0f};

        return overlap_x1 && overlap_x2 && overlap_y;
    }

    inline void entity_layout::resolve_collision(const sprite_bank& bank,
            const sprite& a, const sys::index_t ai, float& ax, float& ay,
            float& az) noexcept
    {
        for (sys::index_t i{0}; i < entity_ids.size(); ++i) {
            if (ai == i)
                continue;

            if (const sprite& b{bank[entity_ids[i]]};
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

    inline void entity_layout::update(const sprite_bank& bank, const float dt,
            const float display_width, const float display_height) noexcept
    {
        for (std::size_t i{0}; i < entity_ids.size(); ++i) {
            const entity_id id{entity_ids[i]};
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

            if (next_x + static_cast<float>(sprite.hb_left) < 0.0f) {
                next_x = -static_cast<float>(sprite.hb_left);
                rdx = 0.0f;
            }
            else if (next_x + static_cast<float>(sprite.hb_right) >
                    display_width) {
                next_x = display_width - static_cast<float>(sprite.hb_right);
                rdx = 0.0f;
            }

            if (next_y + static_cast<float>(sprite.hb_top) < 0) {
                next_y = -static_cast<float>(sprite.hb_top);
                rdy = 0.0f;
            }
            else if (next_y + static_cast<float>(sprite.hb_bottom) >
                    display_height) {
                next_y = display_height - static_cast<float>(sprite.hb_bottom);
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
