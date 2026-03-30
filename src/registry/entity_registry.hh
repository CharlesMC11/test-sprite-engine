#ifndef SC_REGISTRY_SCENE_REGISTRY_HH
#define SC_REGISTRY_SCENE_REGISTRY_HH

#include <arm_neon.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <span>

#include "assets/sprite32_index.hh"
#include "core/core.hh"
#include "core/memory.hh"

/**
 * Register a channel accessor method for the registry.
 *
 * @param enum_T
 * The enum referred to by the buffer.
 *
 * @param T
 * The type contained in the buffer.
 *
 * @param enum_val
 * The enum value to access.
 *
 * @param buffer
 * The name of the buffer.
 */
#define REGISTER_ACCESSOR(enum_T, T, enum_val, buffer)                         \
    [[nodiscard]] constexpr auto enum_val##_ptr() noexcept -> T* __restrict    \
    {                                                                          \
        return get_ptr<enum_T, T, false>(buffer, enum_T::enum_val);            \
    }                                                                          \
                                                                               \
    [[nodiscard]] constexpr auto enum_val##_ptr() const noexcept               \
            -> const T* __restrict                                             \
    {                                                                          \
        return get_ptr<enum_T, T, true>(buffer, enum_T::enum_val);             \
    }

#define REGISTER_XFORM_CHANNEL_ACCESSOR(enum_val)                              \
    REGISTER_ACCESSOR(xform_channel, float, enum_val, float_buffer_)

#define REGISTER_INDEX_CHANNEL_ACCESSOR(enum_val)                              \
    REGISTER_ACCESSOR(index_channel, core::index_t, enum_val, index_buffer_)

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
            sprite32_index,
            physics_order,
            draw_order,
            count
        };

        // Constructors

        [[nodiscard]] explicit constexpr entity_registry(MTL::Device* device);

        entity_registry(const entity_registry&) = delete;
        entity_registry& operator=(const entity_registry&) = delete;

        entity_registry(entity_registry&&) = default;
        entity_registry& operator=(entity_registry&&) noexcept = default;

        ~entity_registry() = default;

        // Public methods

        /**
         * Update the current layout.
         *
         * @param dt
         * The delta time.
         */
        constexpr void update(float dt) noexcept;

        /**
         * Commit the changes to the registry.
         */
        constexpr void commit() noexcept;

        /**
         * Sort the draw order based on screen coordinates.
         */
        constexpr void sort_draw() noexcept;

        /**
         * Print the entries in the registry.
         */
        void print() const;

        // Mutators

        /**
         * Reserve space in the registry.
         *
         * @param n
         * The number of entries to reserve.
         */
        constexpr void reserve(std::size_t n);

        /**
         * Add a new entity to the layout.
         *
         * @param start_x
         * The starting horizontal position.
         *
         * @param start_y
         * The starting vertical position.
         *
         * @param start_z
         * The starting aerial position.
         *
         * @param i
         * The entity's index in the atlas.
         */
        constexpr void spawn(float start_x, float start_y, float start_z,
                sprites::sprite32_index i);

        // Accessors

        [[nodiscard]] constexpr auto xform_buffer() const noexcept
                -> const MTL::Buffer* __restrict;

        [[nodiscard]] constexpr auto index_buffer() const noexcept
                -> const MTL::Buffer* __restrict;

        REGISTER_XFORM_CHANNEL_ACCESSOR(pos_x)
        REGISTER_XFORM_CHANNEL_ACCESSOR(pos_y)
        REGISTER_XFORM_CHANNEL_ACCESSOR(pos_z)

        REGISTER_XFORM_CHANNEL_ACCESSOR(vec_x)
        REGISTER_XFORM_CHANNEL_ACCESSOR(vec_y)
        REGISTER_XFORM_CHANNEL_ACCESSOR(vec_z)

        REGISTER_XFORM_CHANNEL_ACCESSOR(new_x)
        REGISTER_XFORM_CHANNEL_ACCESSOR(new_y)
        REGISTER_XFORM_CHANNEL_ACCESSOR(new_z)

        REGISTER_INDEX_CHANNEL_ACCESSOR(sprite32_index)
        REGISTER_INDEX_CHANNEL_ACCESSOR(physics_order)
        REGISTER_INDEX_CHANNEL_ACCESSOR(draw_order)

        [[nodiscard]] constexpr std::size_t count() const noexcept;
        [[nodiscard]] constexpr std::size_t capacity() const noexcept;

        [[nodiscard]] constexpr std::size_t offset(
                xform_channel) const noexcept;

        [[nodiscard]] constexpr std::size_t offset(
                index_channel) const noexcept;

        // Attributes

        bool needs_sort{false};

    private:
        // Type aliases

        template<typename T, bool IsConst>
        using ptr_t = std::conditional_t<IsConst, const T*, T*>;

        // Static methods

        template<typename E, typename T, bool IsConst>
        [[nodiscard]] static constexpr auto get_ptr(
                const mem::channel_pool<T, static_cast<std::size_t>(E::count)>&
                        buffer,
                E channel) noexcept -> ptr_t<T, IsConst> __restrict;

        // Attributes

        mem::channel_pool<float, static_cast<std::size_t>(xform_channel::count)>
                float_buffer_;
        mem::channel_pool<core::index_t,
                static_cast<std::size_t>(index_channel::count)>
                index_buffer_;
        MTL::Device* device_{nullptr};
    };

    // Constructors

    constexpr entity_registry::entity_registry(MTL::Device* device)
        : device_{device}
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
        float_buffer_.grow(device_, n);
        index_buffer_.grow(device_, n);
    }

    constexpr void entity_registry::spawn(const float start_x,
            const float start_y, const float start_z,
            const sprites::sprite32_index i)
    {
        if (float_buffer_.capacity <= float_buffer_.count) [[unlikely]]
            reserve(std::max(static_cast<std::size_t>(core::kCacheAlignment),
                    capacity() * 2u));

        const auto idx{static_cast<core::index_t>(count())};

        pos_x_ptr()[idx] = new_x_ptr()[idx] = start_x;
        pos_y_ptr()[idx] = new_y_ptr()[idx] = start_y;
        pos_z_ptr()[idx] = new_z_ptr()[idx] = start_z;

        vec_x_ptr()[idx] = vec_y_ptr()[idx] = vec_z_ptr()[idx] = 0.0f;

        sprite32_index_ptr()[idx] = static_cast<core::index_t>(i);
        physics_order_ptr()[idx] = draw_order_ptr()[idx] = idx;

        ++float_buffer_.count;
        ++index_buffer_.count;
        needs_sort = true;
    }

    // Accessors

    [[nodiscard]] constexpr std::size_t entity_registry::count() const noexcept
    {
        return float_buffer_.count;
    }

    [[nodiscard]] constexpr std::size_t
    entity_registry::capacity() const noexcept
    {
        return float_buffer_.capacity;
    }

    [[nodiscard]] constexpr std::size_t entity_registry::offset(
            xform_channel channel) const noexcept
    {
        return float_buffer_.subarray_offset(static_cast<std::size_t>(channel));
    }

    [[nodiscard]] constexpr std::size_t entity_registry::offset(
            index_channel channel) const noexcept
    {
        return index_buffer_.subarray_offset(static_cast<std::size_t>(channel));
    }

    [[nodiscard]] constexpr auto entity_registry::xform_buffer() const noexcept
            -> const MTL::Buffer* __restrict
    {
        return float_buffer_.buffer.get();
    }

    [[nodiscard]] constexpr auto entity_registry::index_buffer() const noexcept
            -> const MTL::Buffer* __restrict
    {
        return index_buffer_.buffer.get();
    }

    // Private helpers

    template<typename Channel, typename T, bool IsConst>
    [[nodiscard]] constexpr auto entity_registry::get_ptr(
            const mem::channel_pool<T,
                    static_cast<std::size_t>(Channel::count)>& buffer,
            Channel channel) noexcept -> ptr_t<T, IsConst> __restrict
    {
        return buffer[static_cast<std::size_t>(channel)];
    }

} // namespace sc

#undef REGISTER_INDEX_CHANNEL_ACCESSOR
#undef REGISTER_XFORM_CHANNEL_ACCESSOR
#undef REGISTER_ACCESSOR

#endif // SC_REGISTRY_SCENE_REGISTRY_HH
