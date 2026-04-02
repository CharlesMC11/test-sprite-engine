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
            const float32x4_t v_pos_x{vld1q_f32(&x_pos_ptr()[i])};
            const float32x4_t v_pos_y{vld1q_f32(&y_pos_ptr()[i])};
            const float32x4_t v_pos_z{vld1q_f32(&z_pos_ptr()[i])};

            const float32x4_t v_vel_x{vld1q_f32(&x_vel_ptr()[i])};
            const float32x4_t v_vel_y{vld1q_f32(&y_vel_ptr()[i])};
            const float32x4_t v_vel_z{vld1q_f32(&z_vel_ptr()[i])};

            const float32x4_t v_new_x{vfmaq_f32(v_pos_x, v_vel_x, v_dt)};
            const float32x4_t v_new_y{vfmaq_f32(v_pos_y, v_vel_y, v_dt)};
            const float32x4_t v_new_z{vfmaq_f32(v_pos_z, v_vel_z, v_dt)};

            vst1q_f32(&new_x_pos_ptr()[i], v_new_x);
            vst1q_f32(&new_y_pos_ptr()[i], v_new_y);
            vst1q_f32(&new_z_pos_ptr()[i], v_new_z);
        }

        for (std::size_t i{vectorized_lim}; i < n; ++i) {
            new_x_pos_ptr()[i] = x_pos_ptr()[i] + x_vel_ptr()[i] * dt;
            new_y_pos_ptr()[i] = y_pos_ptr()[i] + y_vel_ptr()[i] * dt;
            new_z_pos_ptr()[i] = z_pos_ptr()[i] + z_vel_ptr()[i] * dt;
        }
    }

    void entity_registry::sort_draw() noexcept
    {
        if (draw_order_needs_sort) {
            std::span tmp{draw_order_ptr(), count()};

            const float* __restrict y_ptr{y_pos_ptr()};
            const float* __restrict z_ptr{z_pos_ptr()};

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

    // Mutators

    void entity_registry::reserve(const std::size_t n)
    {
        xform_buffer_.grow(device_, n);
        index_buffer_.grow(device_, n);
    }

    void entity_registry::spawn(
            const core::index_t i, const float x, const float y, const float z)
    {
        if (xform_buffer_.capacity <= xform_buffer_.count) [[unlikely]]
            reserve(std::max(static_cast<std::size_t>(core::kCacheAlignment),
                    capacity() * 2UZ));

        const std::size_t idx{count()};

        x_pos_ptr()[idx] = new_x_pos_ptr()[idx] = x;
        y_pos_ptr()[idx] = new_y_pos_ptr()[idx] = y;
        z_pos_ptr()[idx] = new_z_pos_ptr()[idx] = z;

        x_vel_ptr()[idx] = y_vel_ptr()[idx] = z_vel_ptr()[idx] = 0.0f;

        sprite_index_ptr()[idx] = i;
        draw_order_ptr()[idx] = static_cast<core::index_t>(idx);

        ++xform_buffer_.count;
        ++index_buffer_.count;
        draw_order_needs_sort = true;
    }

    void entity_registry::spawn(const assets::sprite32_index i, const float x,
            const float y, const float z)
    {
        spawn(static_cast<core::index_t>(i), x, y, z);
    }

} // namespace sc

std::ostream& operator<<(std::ostream& out, const sc::entity_registry& registry)
{
    for (sc::core::index_t i{0U}; i < registry.count(); ++i) {
        out << std::format(
                "Entity {} (Sprite Index: {})\n\tpos ({:7.2f}, {:7.2f}, "
                "{:7.2f})\n\tvec <{:7.2f}, {:7.2f}, {:7.2f}>\n\n",
                i, registry.sprite_index_ptr()[i], registry.x_pos_ptr()[i],
                registry.y_pos_ptr()[i], registry.z_pos_ptr()[i],
                registry.x_vel_ptr()[i], registry.y_vel_ptr()[i],
                registry.z_vel_ptr()[i]);
    }

    return out;
}
