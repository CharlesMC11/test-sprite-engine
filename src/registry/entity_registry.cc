#include "registry/entity_registry.hh"

#include <arm_neon.h>

#include <cstddef>
#include <format>
#include <iostream>
#include <span>

namespace sc {

    // Public methods

    void entity_registry::update(const float dt) noexcept
    {
        const std::size_t n{count()};
        const std::size_t vectorized_lim{n - n % 4UZ};

        const float32x4_t v_dt{vdupq_n_f32(dt)};
        for (std::size_t i{0UZ}; i < vectorized_lim; i += 4UZ) {
            const float32x4_t v_pos_x{vld1q_f32(&pos_x_ptr()[i])};
            const float32x4_t v_pos_y{vld1q_f32(&pos_y_ptr()[i])};
            const float32x4_t v_pos_z{vld1q_f32(&pos_z_ptr()[i])};

            const float32x4_t v_vel_x{vld1q_f32(&vel_x_ptr()[i])};
            const float32x4_t v_vel_y{vld1q_f32(&vel_y_ptr()[i])};
            const float32x4_t v_vel_z{vld1q_f32(&vel_z_ptr()[i])};

            const float32x4_t v_new_x{vfmaq_f32(v_pos_x, v_vel_x, v_dt)};
            const float32x4_t v_new_y{vfmaq_f32(v_pos_y, v_vel_y, v_dt)};
            const float32x4_t v_new_z{vfmaq_f32(v_pos_z, v_vel_z, v_dt)};

            vst1q_f32(&new_x_ptr()[i], v_new_x);
            vst1q_f32(&new_y_ptr()[i], v_new_y);
            vst1q_f32(&new_z_ptr()[i], v_new_z);
        }

        for (std::size_t i{vectorized_lim}; i < n; ++i) {
            new_x_ptr()[i] = pos_x_ptr()[i] + vel_x_ptr()[i] * dt;
            new_y_ptr()[i] = pos_y_ptr()[i] + vel_y_ptr()[i] * dt;
            new_z_ptr()[i] = pos_z_ptr()[i] + vel_z_ptr()[i] * dt;
        }
    }

    void entity_registry::commit() noexcept
    {
        const std::size_t n{count()};
        const std::size_t vectorized_lim{n - n % 4UZ};

        for (std::size_t i{0UZ}; i < vectorized_lim; i += 4UZ) {
            vst1q_f32(&pos_x_ptr()[i], vld1q_f32(&new_x_ptr()[i]));
            vst1q_f32(&pos_y_ptr()[i], vld1q_f32(&new_y_ptr()[i]));
            vst1q_f32(&pos_z_ptr()[i], vld1q_f32(&new_z_ptr()[i]));
        }

        for (std::size_t i{vectorized_lim}; i < n; ++i) {
            pos_x_ptr()[i] = new_x_ptr()[i];
            pos_y_ptr()[i] = new_y_ptr()[i];
            pos_z_ptr()[i] = new_z_ptr()[i];
        }
    }

    void entity_registry::sort_draw() noexcept
    {
        if (draw_order_needs_sort) {
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

            draw_order_needs_sort = false;
        }
    }

    void entity_registry::print() const
    {
        for (std::size_t i{0UZ}; i < count(); ++i) {
            std::cout << std::format(
                    "Entity {} (Sprite Index: {})\n\tpos ({:7.2f}, {:7.2f}, "
                    "{:7.2f})\n\tvec <{:7.2f}, {:7.2f}, {:7.2f}>\n\n",
                    i, static_cast<core::index_t>(sprite32_index_ptr()[i]),
                    pos_x_ptr()[i], pos_y_ptr()[i], pos_z_ptr()[i],
                    vel_x_ptr()[i], vel_y_ptr()[i], vel_z_ptr()[i]);
        }
    }

    // Mutators

    void entity_registry::reserve(const std::size_t n)
    {
        xform_buffer_.grow(device_, n);
        index_buffer_.grow(device_, n);
    }

    void entity_registry::spawn(const float start_x, const float start_y,
            const float start_z, const assets::sprite32_index i)
    {
        if (xform_buffer_.capacity <= xform_buffer_.count) [[unlikely]]
            reserve(std::max(static_cast<std::size_t>(core::kCacheAlignment),
                    capacity() * 2UZ));

        const auto idx{static_cast<core::index_t>(count())};

        pos_x_ptr()[idx] = new_x_ptr()[idx] = start_x;
        pos_y_ptr()[idx] = new_y_ptr()[idx] = start_y;
        pos_z_ptr()[idx] = new_z_ptr()[idx] = start_z;

        vel_x_ptr()[idx] = vel_y_ptr()[idx] = vel_z_ptr()[idx] = 0.0f;

        sprite32_index_ptr()[idx] = static_cast<core::index_t>(i);
        draw_order_ptr()[idx] = idx;

        ++xform_buffer_.count;
        ++index_buffer_.count;
        draw_order_needs_sort = true;
    }

} // namespace sc
