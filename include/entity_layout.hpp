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
        std::vector<float> x, y, dx, dy;
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
         * @brief
         * @param n
         */
        void reserve(std::size_t n) noexcept;

        /**
         * @brief
         * @param dt
         * @param screen_width
         * @param screen_height
         */
        void update(float dt, float screen_width, float screen_height,
                const sprite_bank& bank) noexcept;

        /**
         * @brief
         * @param start_x
         * @param start_y
         * @param id
         */
        void spawn(float start_x, float start_y, entity_id id) noexcept;

    private:
        bool needs_sort_{false};
    };

    inline entity_layout::entity_layout(
            const std::size_t reserve_count) noexcept
    {
        this->reserve(reserve_count < sys::ALIGNMENT ? sys::ALIGNMENT
                                                     : reserve_count);
    }

    inline void entity_layout::reserve(const std::size_t n) noexcept
    {
        const std::size_t aligned{
                (n + sys::ALIGNMENT - 1) & ~(sys::ALIGNMENT - 1)};

        if (aligned > x.capacity()) {
            x.reserve(aligned);
            y.reserve(aligned);
            dx.reserve(aligned);
            dy.reserve(aligned);
            entity_ids.reserve(aligned);
            draw_order.reserve(aligned);
        }
    }

    [[nodiscard]] constexpr std::size_t entity_layout::size() const noexcept
    {
        return x.size();
    }

    inline void entity_layout::update(const float dt, const float screen_width,
            const float screen_height, const sprite_bank& bank) noexcept
    {
        for (std::size_t i{0}; i < x.size(); ++i) {
            const auto& sprite{bank[entity_ids[i]]};

            float next_x{x[i] + dx[i] * dt};
            float next_y{y[i] + dy[i] * dt};

            if (next_x + static_cast<float>(sprite.hb_min_x) < 0.0f) {
                next_x = -static_cast<float>(sprite.hb_min_x);
                dx[i] = 0.0f;
            }
            else if (next_x + sprite.hb_max_x > screen_width) {
                next_x = screen_width - static_cast<float>(sprite.hb_max_x);
                dx[i] = 0.0f;
            }

            if (next_y + static_cast<float>(sprite.hb_min_y) < 0) {
                next_y = -static_cast<float>(sprite.hb_min_y);
                dy[i] = 0.0f;
            }
            else if (next_y + sprite.hb_max_y > screen_height) {
                next_y = screen_height - static_cast<float>(sprite.hb_max_y);
                dy[i] = 0.0f;
            }

            x[i] = next_x;
            y[i] = next_y;

            if (std::abs(dy[i]) > 0.001f)
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

    inline void entity_layout::spawn(const float start_x, const float start_y,
            const entity_id id) noexcept
    {
        if (x.capacity() == x.size()) [[unlikely]]
            this->reserve(x.size() * 2);

        x.push_back(start_x);
        y.push_back(start_y);
        dx.push_back(0.0f);
        dy.push_back(0.0f);
        entity_ids.push_back(id);
        draw_order.push_back(static_cast<std::uint32_t>(x.size() - 1));

        needs_sort_ = true;
    }

} // namespace sc
