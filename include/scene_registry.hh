/**
 * @file scene_registry.hh
 * @brief
 */
#pragma once

#include <arm_neon.h>

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "core.hh"
#include "memory.hh"
#include "sprite_index.hh"

namespace sc {

    /**
     * @struct scene_registry
     * @brief
     */
    class scene_registry final {
    public:
        explicit constexpr scene_registry(
                std::size_t reserve_count = core::kCacheAlignment) noexcept;

        scene_registry(const scene_registry&) = delete;
        scene_registry(scene_registry&&) = default;

        ~scene_registry() = default;

        scene_registry& operator=(const scene_registry&) = delete;
        scene_registry& operator=(scene_registry&&) = default;

        /**
         * @brief Reserve space in the registry.
         * @param n The number of entries to reserve.
         */
        constexpr void reserve(std::size_t n) noexcept;

        void print() const noexcept;

        /**
         * @brief Add a new entity to the layout.
         * @param start_x The starting horizontal position.
         * @param start_y The starting vertical position.
         * @param start_z The starting aerial position.
         * @param i The entity's ID.
         */
        constexpr void spawn(float start_x, float start_y, float start_z,
                sprites::sprite_index i) noexcept;

        /**
         * @brief Update the current layout.
         * @param dt The delta time.
         */
        constexpr void update(float dt) noexcept;

        constexpr void commit() noexcept;

        constexpr void sort_draw() noexcept;

        [[nodiscard]] constexpr float* pos_x() const noexcept;
        [[nodiscard]] constexpr float* pos_y() const noexcept;
        [[nodiscard]] constexpr float* pos_z() const noexcept;

        [[nodiscard]] constexpr float* vec_x() const noexcept;
        [[nodiscard]] constexpr float* vec_y() const noexcept;
        [[nodiscard]] constexpr float* vec_z() const noexcept;

        [[nodiscard]] constexpr float* new_x() const noexcept;
        [[nodiscard]] constexpr float* new_y() const noexcept;
        [[nodiscard]] constexpr float* new_z() const noexcept;

        [[nodiscard]] constexpr std::size_t count() const noexcept;
        [[nodiscard]] constexpr std::size_t capacity() const noexcept;

        std::vector<sprites::sprite_index> indices;
        std::vector<core::index_t> physics_order;
        std::vector<core::index_t> draw_order;

        bool needs_sort{false};

    private:
        enum class attr : std::size_t {
            x,
            y,
            z,
            vx,
            vy,
            vz,
            next_x,
            next_y,
            next_z,
            count
        };

        static constexpr auto kAttributeCount_{
                static_cast<std::underlying_type_t<attr>>(attr::count)};

        mem::soa_block<float, kAttributeCount_> buffer_;
    };

    constexpr float* scene_registry::pos_x() const noexcept
    {
        return buffer_[static_cast<std::underlying_type_t<attr>>(attr::x)];
    }

    constexpr float* scene_registry::pos_y() const noexcept
    {
        return buffer_[static_cast<std::underlying_type_t<attr>>(attr::y)];
    }

    constexpr float* scene_registry::pos_z() const noexcept
    {
        return buffer_[static_cast<std::underlying_type_t<attr>>(attr::z)];
    }

    constexpr float* scene_registry::vec_x() const noexcept
    {
        return buffer_[static_cast<std::underlying_type_t<attr>>(attr::vx)];
    }

    constexpr float* scene_registry::vec_y() const noexcept
    {
        return buffer_[static_cast<std::underlying_type_t<attr>>(attr::vy)];
    }

    constexpr float* scene_registry::vec_z() const noexcept
    {
        return buffer_[static_cast<std::underlying_type_t<attr>>(attr::vz)];
    }

    constexpr float* scene_registry::new_x() const noexcept
    {
        return buffer_[static_cast<std::underlying_type_t<attr>>(attr::next_x)];
    }

    constexpr float* scene_registry::new_y() const noexcept
    {
        return buffer_[static_cast<std::underlying_type_t<attr>>(attr::next_y)];
    }

    constexpr float* scene_registry::new_z() const noexcept
    {
        return buffer_[static_cast<std::underlying_type_t<attr>>(attr::next_z)];
    }

    constexpr std::size_t scene_registry::count() const noexcept
    {
        return buffer_.count;
    }

    constexpr std::size_t scene_registry::capacity() const noexcept
    {
        return buffer_.capacity;
    }

    constexpr scene_registry::scene_registry(
            const std::size_t reserve_count) noexcept
    {
        reserve(reserve_count < core::kCacheAlignment ? core::kCacheAlignment
                                                      : reserve_count);
    }

    constexpr void scene_registry::reserve(const std::size_t n) noexcept
    {
        buffer_.grow(n);
        indices.reserve(buffer_.capacity);
        physics_order.reserve(buffer_.capacity);
        draw_order.reserve(buffer_.capacity);
    }

    constexpr void scene_registry::spawn(const float start_x,
            const float start_y, const float start_z,
            const sprites::sprite_index i) noexcept
    {
        if (buffer_.capacity >= buffer_.count) [[unlikely]]
            reserve(std::max(static_cast<std::size_t>(core::kCacheAlignment),
                    capacity() * 2u));

        pos_x()[count()] = start_x;
        pos_y()[count()] = start_y;
        pos_z()[count()] = start_z;
        vec_x()[count()] = 0.0f;
        vec_y()[count()] = 0.0f;
        vec_z()[count()] = 0.0f;
        new_x()[count()] = start_x;
        new_y()[count()] = start_y;
        new_z()[count()] = start_z;
        indices.push_back(i);
        physics_order.push_back(static_cast<core::index_t>(count()));
        draw_order.push_back(static_cast<core::index_t>(count()));

        ++buffer_.count;

        needs_sort = true;
    }

    constexpr void scene_registry::update(const float dt) noexcept
    {
        const auto n{static_cast<core::index_t>(count())};
        const core::index_t vectorized_lim{n - n % 4u};

        if (vectorized_lim > 0u) {
            const float32x4_t v_dt{vdupq_n_f32(dt)};
            for (core::index_t i{0u}; i < vectorized_lim; i += 4u) {
                const float32x4_t v_x{vld1q_f32(&pos_x()[i])};
                const float32x4_t v_y{vld1q_f32(&pos_y()[i])};
                const float32x4_t v_z{vld1q_f32(&pos_z()[i])};

                const float32x4_t v_vx{vld1q_f32(&vec_x()[i])};
                const float32x4_t v_vy{vld1q_f32(&vec_y()[i])};
                const float32x4_t v_vz{vld1q_f32(&vec_z()[i])};

                const float32x4_t v_next_x{vfmaq_f32(v_x, v_vx, v_dt)};
                const float32x4_t v_next_y{vfmaq_f32(v_y, v_vy, v_dt)};
                const float32x4_t v_next_z{vfmaq_f32(v_z, v_vz, v_dt)};

                vst1q_f32(&new_x()[i], v_next_x);
                vst1q_f32(&new_y()[i], v_next_y);
                vst1q_f32(&new_z()[i], v_next_z);
            }
        }

        for (core::index_t i{vectorized_lim}; i < n; ++i) {
            new_x()[i] = pos_x()[i] + vec_x()[i] * dt;
            new_y()[i] = pos_y()[i] + vec_y()[i] * dt;
            new_z()[i] = pos_z()[i] + vec_z()[i] * dt;
        }
    }

    constexpr void scene_registry::commit() noexcept
    {
        const std::size_t size{sizeof(float) * count()};

        std::memcpy(pos_x(), new_x(), size);
        std::memcpy(pos_y(), new_y(), size);
        std::memcpy(pos_z(), new_z(), size);
    }

    constexpr void scene_registry::sort_draw() noexcept
    {
        if (needs_sort) {
            std::ranges::sort(draw_order.begin(), draw_order.end(),
                    [&](const core::index_t a, const core::index_t b) noexcept
                            -> bool { return pos_y()[a] < pos_y()[b]; });

            needs_sort = false;
        }
    }

} // namespace sc
