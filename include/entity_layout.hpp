/**
 * @file entity_layout.hpp
 * @brief
 */
#pragma once

#include <algorithm>
#include <vector>

#include "constants.hpp"
#include "entity_id.hpp"
#include "sprite.hpp"

namespace sc {

    /**
     * @struct entity_layout
     * @brief
     */
    struct entity_layout final {

        /// TODO: Make custom allocator
        std::vector<float> x, y, dx, dy;
        std::vector<entity_id> entity_ids;

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
        void update(float dt, float screen_width, float screen_height) noexcept;

        /**
         * @brief
         * @param start_x
         * @param start_y
         * @param id
         */
        void spawn(float start_x, float start_y, entity_id id) noexcept;
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
        }
    }

    [[nodiscard]] constexpr std::size_t entity_layout::size() const noexcept
    {
        return x.size();
    }

    inline void entity_layout::update(const float dt, const float screen_width,
            const float screen_height) noexcept
    {
        const auto max_x{screen_width - static_cast<float>(SPRITE_WIDTH)};
        const auto max_y{screen_height - static_cast<float>(SPRITE_HEIGHT)};

        /// TODO: Use NEON / assembly
        for (std::size_t i{0}; i < x.size(); ++i) {
            x[i] = std::clamp(x[i] + dx[i] * dt, 0.0f, max_x);
            y[i] = std::clamp(y[i] + dy[i] * dt, 0.0f, max_y);
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
    }

} // namespace sc
