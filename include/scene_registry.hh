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

#include "atlas_index.hh"
#include "core.hh"

namespace sc {

    /**
     * @struct scene_registry
     * @brief
     */
    class scene_registry final {
    public:
        explicit constexpr scene_registry(
                std::size_t reserve_count = core::kAlignment) noexcept;

        scene_registry(const scene_registry&) = delete;
        scene_registry(scene_registry&&) = default;

        ~scene_registry() = default;

        scene_registry& operator=(const scene_registry&) = delete;
        scene_registry& operator=(scene_registry&&) = default;

        [[nodiscard]] constexpr std::size_t count() const noexcept;

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
                sprites::atlas_index i) noexcept;

        /**
         * @brief Update the current layout.
         * @param dt The delta time.
         */
        constexpr void update(float dt) noexcept;

        constexpr void commit() noexcept;

        constexpr void sort_draw() noexcept;

        [[nodiscard]] constexpr float* x() const noexcept;
        [[nodiscard]] constexpr float* y() const noexcept;
        [[nodiscard]] constexpr float* z() const noexcept;
        [[nodiscard]] constexpr float* vx() const noexcept;
        [[nodiscard]] constexpr float* vy() const noexcept;
        [[nodiscard]] constexpr float* vz() const noexcept;
        [[nodiscard]] constexpr float* next_x() const noexcept;
        [[nodiscard]] constexpr float* next_y() const noexcept;
        [[nodiscard]] constexpr float* next_z() const noexcept;

        std::vector<sprites::atlas_index> indices;
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

        struct aligned_deleter {
            void operator()(void* ptr) const { std::free(ptr); }
        };

        static constexpr auto kAttributeCount_{
                static_cast<std::size_t>(attr::count)};

        std::unique_ptr<float[], aligned_deleter> buffer_{nullptr};
        std::size_t count_{0u};
        std::size_t capacity_{0u};
    };

    constexpr float* scene_registry::x() const noexcept
    {
        return buffer_.get();
    }

    constexpr float* scene_registry::y() const noexcept
    {
        return buffer_.get() + capacity_;
    }

    constexpr float* scene_registry::z() const noexcept
    {
        return buffer_.get() + capacity_ * 2;
    }

    constexpr float* scene_registry::vx() const noexcept
    {
        return buffer_.get() + capacity_ * 3;
    }

    constexpr float* scene_registry::vy() const noexcept
    {
        return buffer_.get() + capacity_ * 4;
    }

    constexpr float* scene_registry::vz() const noexcept
    {
        return buffer_.get() + capacity_ * 5;
    }

    constexpr float* scene_registry::next_x() const noexcept
    {
        return buffer_.get() + capacity_ * 6;
    }

    constexpr float* scene_registry::next_y() const noexcept
    {
        return buffer_.get() + capacity_ * 7;
    }

    constexpr float* scene_registry::next_z() const noexcept
    {
        return buffer_.get() + capacity_ * 8;
    }

    constexpr scene_registry::scene_registry(
            const std::size_t reserve_count) noexcept
    {
        reserve(reserve_count < core::kAlignment ? core::kAlignment
                                                 : reserve_count);
    }

    [[nodiscard]] constexpr std::size_t scene_registry::count() const noexcept
    {
        return count_;
    }

    constexpr void scene_registry::reserve(const std::size_t n) noexcept
    {
        const std::size_t aligned{
                n + core::kAlignment - 1u & ~(core::kAlignment - 1u)};
        if (aligned <= capacity_)
            return;

        const std::size_t new_size_bytes{
                sizeof(float) * aligned * kAttributeCount_};

        float* old_buffer{buffer_.get()};
        auto* new_buffer{
                static_cast<float*>(std::realloc(old_buffer, new_size_bytes))};
        if (!new_buffer) {
            return;
        }

        for (std::int_fast8_t i{kAttributeCount_ - 1}; i >= 0; --i) {
            const float* src{old_buffer + i * capacity_};
            if (float* dst{new_buffer + i * aligned}; src != dst)
                std::memmove(dst, src, sizeof(float) * count_);
        }

        if (old_buffer != new_buffer) {
            buffer_.release();
            buffer_.reset(new_buffer);
        }
        capacity_ = aligned;

        indices.reserve(aligned);
        physics_order.reserve(aligned);
        draw_order.reserve(aligned);
    }

    constexpr void scene_registry::spawn(const float start_x,
            const float start_y, const float start_z,
            const sprites::atlas_index i) noexcept
    {
        if (capacity_ == count_) [[unlikely]]
            reserve(count_ * 2u);

        x()[count_] = start_x;
        y()[count_] = start_y;
        z()[count_] = start_z;
        vx()[count_] = 0.0f;
        vy()[count_] = 0.0f;
        vz()[count_] = 0.0f;
        next_x()[count_] = start_x;
        next_y()[count_] = start_y;
        next_z()[count_] = start_z;
        indices.push_back(i);
        physics_order.push_back(static_cast<core::index_t>(count_));
        draw_order.push_back(static_cast<core::index_t>(count_));

        ++count_;

        needs_sort = true;
    }

    constexpr void scene_registry::update(const float dt) noexcept
    {
        const auto n{static_cast<core::index_t>(count_)};
        const core::index_t vectorized_lim{n - n % 4u};

        if (vectorized_lim > 0u) {
            const float32x4_t v_dt{vdupq_n_f32(dt)};
            for (core::index_t i{0u}; i < vectorized_lim; i += 4u) {
                const float32x4_t v_x{vld1q_f32(&x()[i])};
                const float32x4_t v_y{vld1q_f32(&y()[i])};
                const float32x4_t v_z{vld1q_f32(&z()[i])};

                const float32x4_t v_vx{vld1q_f32(&vx()[i])};
                const float32x4_t v_vy{vld1q_f32(&vy()[i])};
                const float32x4_t v_vz{vld1q_f32(&vz()[i])};

                const float32x4_t v_next_x{vfmaq_f32(v_x, v_vx, v_dt)};
                const float32x4_t v_next_y{vfmaq_f32(v_y, v_vy, v_dt)};
                const float32x4_t v_next_z{vfmaq_f32(v_z, v_vz, v_dt)};

                vst1q_f32(&next_x()[i], v_next_x);
                vst1q_f32(&next_y()[i], v_next_y);
                vst1q_f32(&next_z()[i], v_next_z);
            }
        }

        for (core::index_t i{vectorized_lim}; i < n; ++i) {
            next_x()[i] = x()[i] + vx()[i] * dt;
            next_y()[i] = y()[i] + vy()[i] * dt;
            next_z()[i] = z()[i] + vz()[i] * dt;
        }
    }

    constexpr void scene_registry::commit() noexcept
    {
        std::memcpy(x(), next_x(), sizeof(float) * count_ * 3);
    }

    constexpr void scene_registry::sort_draw() noexcept
    {
        if (needs_sort) {
            std::ranges::sort(draw_order.begin(), draw_order.end(),
                    [&](const core::index_t a, const core::index_t b) noexcept
                            -> bool { return y()[a] < y()[b]; });

            needs_sort = false;
        }
    }

} // namespace sc
