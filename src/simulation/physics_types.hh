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

    struct aabb final {
        // Constructors

        [[nodiscard]] static constexpr aabb from_registry(
                const entity_registry& registry, core::index_t i,
                assets::sprites::metadata meta) noexcept;

        [[nodiscard]] explicit constexpr aabb(float x_pos, float y_pos,
                float z_pos, geometry::bbox<float> bbox, float depth,
                float x_vel, float y_vel, float z_vel) noexcept;

        // Attributes

        geometry::bbox<float> bbox;

        float west{0.0f};
        float east{0.0f};

        float north{0.0f};
        float south{0.0f};

        float nadir{0.f};
        float zenith{0.0f};

        float x_vel{0.0f};
        float y_vel{0.0f};
        float z_vel{0.0f};
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
                registry.z_pos_ptr()[i],
                static_cast<geometry::bbox<float>>(meta.bbox),
                static_cast<float>(meta.depth) / 2.0f, registry.x_vel_ptr()[i],
                registry.y_vel_ptr()[i], registry.z_vel_ptr()[i]};
    }

    [[nodiscard]] constexpr aabb::aabb(const float x_pos, const float y_pos,
            const float z_pos, const geometry::bbox<float> bbox,
            const float depth, const float x_vel, const float y_vel,
            const float z_vel) noexcept
        : bbox{bbox}, west{x_pos + bbox.u_min}, east{x_pos + bbox.u_max},
          north{y_pos - depth}, south{y_pos + depth}, nadir{z_pos},
          zenith{z_pos + bbox.height()}, x_vel{x_vel}, y_vel{y_vel},
          z_vel{z_vel}
    {
    }

} // namespace sc::physics

SC_ENABLE_ENUM_BITWISE_OPS(sc::physics::type)

#endif // SC_SIMULATION_PHYSICS_TYPES_HH
