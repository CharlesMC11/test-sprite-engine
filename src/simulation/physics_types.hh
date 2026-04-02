#ifndef SC_SIMULATION_PHYSICS_TYPES_HH
#define SC_SIMULATION_PHYSICS_TYPES_HH

#include "assets/sprite.hh"
#include "core/core.hh"
#include "math/bbox.hh"
#include "registry/entity_registry.hh"

namespace sc::physics {

    static constexpr float kGravity{9.8f};
    static constexpr float kFixedTimestep{1.0f / 120.0f};
    static constexpr float kMaxVelocity{500.0f};

    static constexpr float kYCollisionDistance{4.0f};

    /**
     * The type of physics that affects an entity.
     */
    enum class type : core::physics_t {
        UNDEFINED = 0U,
        NONE = 1U,
        ACTOR = 1U << 1,
        STATIC = 1U << 2,
        SENSOR = 1U << 3,
        PROJECTILE = 1U << 4,
    };

    struct alignas(float) aabb final {
        // Constructors

        [[nodiscard]] static constexpr aabb from_registry(
                const entity_registry& registry, core::index_t i,
                assets::sprites::metadata meta) noexcept;

        [[nodiscard]] explicit constexpr aabb(float pos_x, float pos_y,
                float pos_z, float vel_x, float vel_y, float vel_z,
                geometry::bbox<float> bbox) noexcept;

        // Attributes

        float left{0.0f};
        float right{0.0f};

        float front{0.0f};
        float back{0.0f};

        float top{0.0f};
        float bottom{0.f};

        float x_vel{0.0f};
        float y_vel{0.0f};
        float z_vel{0.0f};

        geometry::bbox<float> bbox;
    };

    struct alignas(core::kNeonAlignment) sweep_result final {
        float time{1.0f};
        float normal_x{0.0f};
        float normal_y{0.0f};
        float normal_z{0.0f};
    };

    // Constructors

    [[nodiscard]] constexpr aabb aabb::from_registry(
            const entity_registry& registry, const core::index_t i,
            const assets::sprites::metadata meta) noexcept
    {
        return aabb{registry.x_pos_ptr()[i], registry.y_pos_ptr()[i],
                registry.z_pos_ptr()[i], registry.x_vel_ptr()[i],
                registry.y_vel_ptr()[i], registry.z_vel_ptr()[i],
                static_cast<geometry::bbox<float>>(meta.bbox)};
    }

    [[nodiscard]] constexpr aabb::aabb(const float pos_x, const float pos_y,
            const float pos_z, const float vel_x, const float vel_y,
            const float vel_z, const geometry::bbox<float> bbox) noexcept
        : left{pos_x + bbox.min_u}, right{pos_x + bbox.max_u + 1.0f},
          front{pos_y + kYCollisionDistance}, back{pos_y - kYCollisionDistance},
          top{pos_z + bbox.height()}, bottom{pos_z}, x_vel{vel_x}, y_vel{vel_y},
          z_vel{vel_z}, bbox{bbox}
    {
    }

} // namespace sc::physics

SC_ENABLE_ENUM_BITWISE_OPS(sc::physics::type)

#endif // SC_SIMULATION_PHYSICS_TYPES_HH
