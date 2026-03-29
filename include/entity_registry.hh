/**
 * @file entity_registry.hh
 * @brief
 */
#pragma once

#include <arm_neon.h>

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <span>
#include <vector>

#include "core.hh"
#include "memory.hh"
#include "sprite32_index.hh"

#define REGISTER_XFORM_CHANNEL_ACCESSOR(enum_val)                              \
    [[nodiscard]] constexpr auto enum_val##_ptr() noexcept                     \
            -> float* __restrict                                               \
    {                                                                          \
        return float_ptr<xform_channel::enum_val, false>();                    \
    }                                                                          \
                                                                               \
    [[nodiscard]] constexpr auto enum_val##_ptr() const noexcept               \
            -> const float* __restrict                                         \
    {                                                                          \
        return float_ptr<xform_channel::enum_val, true>();                     \
    }

#define REGISTER_INDEX_CHANNEL_ACCESSOR(enum_val)                              \
    [[nodiscard]] constexpr auto enum_val##_ptr() noexcept                     \
            -> core::index_t* __restrict                                       \
    {                                                                          \
        return index_ptr<index_channel::enum_val, false>();                    \
    }                                                                          \
                                                                               \
    [[nodiscard]] constexpr auto enum_val##_ptr() const noexcept               \
            -> const core::index_t* __restrict                                 \
    {                                                                          \
        return index_ptr<index_channel::enum_val, true>();                     \
    }

namespace sc {

    /**
     * @struct entity_registry
     * @brief
     */
    class entity_registry final {
    public:
        // Enums

        enum class xform_channel : std::uint8_t {
            pos_x,
            pos_y,
            pos_z,
            vec_x,
            vec_y,
            vec_z,
            new_x,
            new_y,
            new_z,
            count
        };

        enum class index_channel : std::uint8_t {
            physics_order,
            draw_order,
            count
        };

        // Constructors

        [[nodiscard]] explicit constexpr entity_registry();

        entity_registry(const entity_registry&) = delete;
        entity_registry& operator=(const entity_registry&) = delete;

        entity_registry(entity_registry&&) = default;
        entity_registry& operator=(entity_registry&&) = default;

        ~entity_registry() = default;

        // Public methods

        /**
         * @brief Update the current layout.
         * @param dt The delta time.
         */
        constexpr void update(float dt) noexcept;

        /**
         * @brief Commit the changes to the registry.
         */
        constexpr void commit() noexcept;

        /**
         * @brief Sort the draw order based on screen coordinates.
         */
        constexpr void sort_draw() noexcept;

        /**
         * @brief Print the entries in the registry.
         */
        void print() const;

        // Mutators

        /**
         * @brief Reserve space in the registry.
         * @param n The number of entries to reserve.
         */
        constexpr void reserve(std::size_t n);

        /**
         * @brief Add a new entity to the layout.
         * @param start_x The starting horizontal position.
         * @param start_y The starting vertical position.
         * @param start_z The starting aerial position.
         * @param i The entity's index in the atlas.
         */
        constexpr void spawn(float start_x, float start_y, float start_z,
                sprites::sprite32_index i);

        // Accessors

        REGISTER_XFORM_CHANNEL_ACCESSOR(pos_x)
        REGISTER_XFORM_CHANNEL_ACCESSOR(pos_y)
        REGISTER_XFORM_CHANNEL_ACCESSOR(pos_z)

        REGISTER_XFORM_CHANNEL_ACCESSOR(vec_x)
        REGISTER_XFORM_CHANNEL_ACCESSOR(vec_y)
        REGISTER_XFORM_CHANNEL_ACCESSOR(vec_z)

        REGISTER_XFORM_CHANNEL_ACCESSOR(new_x)
        REGISTER_XFORM_CHANNEL_ACCESSOR(new_y)
        REGISTER_XFORM_CHANNEL_ACCESSOR(new_z)

        REGISTER_INDEX_CHANNEL_ACCESSOR(physics_order)
        REGISTER_INDEX_CHANNEL_ACCESSOR(draw_order)

        [[nodiscard]] constexpr std::size_t count() const noexcept;
        [[nodiscard]] constexpr std::size_t capacity() const noexcept;

        // Attributes

        std::vector<sprites::sprite32_index> indices;

        bool needs_sort{false};

    private:
        // Type aliases

        template<typename T, bool IsConst>
        using ptr_t = std::conditional_t<IsConst, const T*, T*>;

        // Accessors

        template<xform_channel Channel, bool IsConst>
        [[nodiscard]] constexpr auto float_ptr() const noexcept
                -> ptr_t<float, IsConst> __restrict;

        template<index_channel Channel, bool IsConst>
        [[nodiscard]] constexpr auto index_ptr() const noexcept
                -> ptr_t<core::index_t, IsConst> __restrict;

        // Attributes

        mem::channel_pool<float, static_cast<std::size_t>(xform_channel::count)>
                float_buffer_;
        mem::channel_pool<core::index_t,
                static_cast<std::size_t>(index_channel::count)>
                index_buffer_;
    };

    // Constructors

    constexpr entity_registry::entity_registry()
    {
        reserve(core::kCacheAlignment);
    }

    // Public methods

    constexpr void entity_registry::update(const float dt) noexcept
    {
        const auto n{static_cast<core::index_t>(count())};
        const core::index_t vectorized_lim{n - n % 4u};

        const float32x4_t v_dt{vdupq_n_f32(dt)};
        for (core::index_t i{0u}; i < vectorized_lim; i += 4u) {
            const float32x4_t v_pos_x{vld1q_f32(&pos_x_ptr()[i])};
            const float32x4_t v_pos_y{vld1q_f32(&pos_y_ptr()[i])};
            const float32x4_t v_pos_z{vld1q_f32(&pos_z_ptr()[i])};

            const float32x4_t v_vec_x{vld1q_f32(&vec_x_ptr()[i])};
            const float32x4_t v_vec_y{vld1q_f32(&vec_y_ptr()[i])};
            const float32x4_t v_vec_z{vld1q_f32(&vec_z_ptr()[i])};

            const float32x4_t v_new_x{vfmaq_f32(v_pos_x, v_vec_x, v_dt)};
            const float32x4_t v_new_y{vfmaq_f32(v_pos_y, v_vec_y, v_dt)};
            const float32x4_t v_new_z{vfmaq_f32(v_pos_z, v_vec_z, v_dt)};

            vst1q_f32(&new_x_ptr()[i], v_new_x);
            vst1q_f32(&new_y_ptr()[i], v_new_y);
            vst1q_f32(&new_z_ptr()[i], v_new_z);
        }

        for (core::index_t i{vectorized_lim}; i < n; ++i) {
            new_x_ptr()[i] = pos_x_ptr()[i] + vec_x_ptr()[i] * dt;
            new_y_ptr()[i] = pos_y_ptr()[i] + vec_y_ptr()[i] * dt;
            new_z_ptr()[i] = pos_z_ptr()[i] + vec_z_ptr()[i] * dt;
        }
    }

    constexpr void entity_registry::commit() noexcept
    {
        const auto n{static_cast<core::index_t>(count())};
        const core::index_t vectorized_lim{n - n % 4u};

        for (core::index_t i{0u}; i < vectorized_lim; i += 4u) {
            vst1q_f32(&pos_x_ptr()[i], vld1q_f32(&new_x_ptr()[i]));
            vst1q_f32(&pos_y_ptr()[i], vld1q_f32(&new_y_ptr()[i]));
            vst1q_f32(&pos_z_ptr()[i], vld1q_f32(&new_z_ptr()[i]));
        }

        for (core::index_t i{vectorized_lim}; i < n; ++i) {
            pos_x_ptr()[i] = new_x_ptr()[i];
            pos_y_ptr()[i] = new_y_ptr()[i];
            pos_z_ptr()[i] = new_z_ptr()[i];
        }
    }

    constexpr void entity_registry::sort_draw() noexcept
    {
        if (needs_sort) {
            std::span tmp{draw_order_ptr(), count()};

            const float* __restrict y_ptr{pos_y_ptr()};
            const float* __restrict z_ptr{pos_z_ptr()};

            std::ranges::sort(tmp.begin(), tmp.end(),
                    [y_ptr, z_ptr](const core::index_t a,
                            const core::index_t b) noexcept -> bool {
                        const float y_a{y_ptr[a]};
                        const float y_b{y_ptr[b]};
                        return std::abs(y_a - y_b) > core::kEpsilon
                                ? y_a < y_b
                                : z_ptr[a] < z_ptr[b];
                    });

            needs_sort = false;
        }
    }

    // Mutators

    constexpr void entity_registry::reserve(const std::size_t n)
    {
        indices.reserve(float_buffer_.capacity);
        float_buffer_.grow(n);
        index_buffer_.grow(n);
    }

    constexpr void entity_registry::spawn(const float start_x,
            const float start_y, const float start_z,
            const sprites::sprite32_index i)
    {
        if (float_buffer_.capacity <= float_buffer_.count) [[unlikely]]
            reserve(std::max(static_cast<std::size_t>(core::kCacheAlignment),
                    capacity() * 2u));

        const auto idx{static_cast<core::index_t>(float_buffer_.count++)};

        indices.push_back(i);
        pos_x_ptr()[idx] = start_x;
        pos_y_ptr()[idx] = start_y;
        pos_z_ptr()[idx] = start_z;
        vec_x_ptr()[idx] = 0.0f;
        vec_y_ptr()[idx] = 0.0f;
        vec_z_ptr()[idx] = 0.0f;
        new_x_ptr()[idx] = start_x;
        new_y_ptr()[idx] = start_y;
        new_z_ptr()[idx] = start_z;
        physics_order_ptr()[idx] = idx;
        draw_order_ptr()[idx] = idx;

        index_buffer_.count = float_buffer_.count;
        needs_sort = true;
    }

    // Accessors

    constexpr std::size_t entity_registry::count() const noexcept
    {
        return float_buffer_.count;
    }

    constexpr std::size_t entity_registry::capacity() const noexcept
    {
        return float_buffer_.capacity;
    }

    // Private methods

    template<entity_registry::xform_channel Channel, bool IsConst>
    [[nodiscard]] constexpr auto entity_registry::float_ptr() const noexcept
            -> ptr_t<float, IsConst> __restrict
    {
        return float_buffer_[static_cast<std::size_t>(Channel)];
    }

    template<entity_registry::index_channel Channel, bool IsConst>
    [[nodiscard]] constexpr auto entity_registry::index_ptr() const noexcept
            -> ptr_t<core::index_t, IsConst> __restrict
    {
        return index_buffer_[static_cast<std::size_t>(Channel)];
    }

} // namespace sc

#undef REGISTER_XFORM_CHANNEL_ACCESSOR
