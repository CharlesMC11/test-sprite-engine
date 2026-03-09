/**
 * @file transform_registry.hpp
 * @brief
 */
#pragma once

#include <algorithm>
#include <vector>

#include "atlas_index.hpp"
#include "constants.hpp"

namespace sc {

    /**
     * @struct transform_registry
     * @brief
     */
    struct transform_registry final {

        /// TODO: Make custom allocator
        std::vector<float> x, y, dx, dy;
        std::vector<atlas_index> sprite_ids;

        explicit transform_registry(
                std::size_t reserve_count = memory::ALIGNMENT) noexcept;

        transform_registry(const transform_registry&) = delete;
        transform_registry(transform_registry&&) = delete;
        transform_registry& operator=(const transform_registry&) = delete;
        transform_registry& operator=(transform_registry&&) = delete;

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
        void add_entity(float start_x, float start_y, atlas_index id) noexcept;
    };

    inline transform_registry::transform_registry(
            const std::size_t reserve_count) noexcept
    {
        this->reserve(reserve_count < memory::ALIGNMENT ? memory::ALIGNMENT
                                                        : reserve_count);
    }

    inline void transform_registry::reserve(const std::size_t n) noexcept
    {
        const std::size_t aligned{
                (n + memory::ALIGNMENT - 1) & ~(memory::ALIGNMENT - 1)};

        if (aligned > x.capacity()) {
            x.reserve(aligned);
            y.reserve(aligned);
            dx.reserve(aligned);
            dy.reserve(aligned);
            sprite_ids.reserve(aligned);
        }
    }

    [[nodiscard]] constexpr std::size_t
    transform_registry::size() const noexcept
    {
        return x.size();
    }

    inline void transform_registry::update(const float dt,
            const float screen_width, const float screen_height) noexcept
    {
        const auto max_x{screen_width - static_cast<float>(SPRITE_WIDTH)};
        const auto max_y{screen_height - static_cast<float>(SPRITE_HEIGHT)};

        /// TODO: Use NEON / assembly
        for (std::size_t i{0}; i < x.size(); ++i) {
            x[i] = std::clamp(x[i] + dx[i] * dt, 0.0f, max_x);
            y[i] = std::clamp(y[i] + dy[i] * dt, 0.0f, max_y);
        }
    }

    inline void transform_registry::add_entity(const float start_x,
            const float start_y, const atlas_index id) noexcept
    {
        if (x.capacity() == x.size()) [[unlikely]]
            this->reserve(x.size() * 2);

        x.push_back(start_x);
        y.push_back(start_y);
        dx.push_back(0.0f);
        dy.push_back(0.0f);
        sprite_ids.push_back(id);
    }

} // namespace sc
