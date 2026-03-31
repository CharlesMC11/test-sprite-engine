#ifndef SC_SIMULATION_PHYSICS_TYPES_HH
#define SC_SIMULATION_PHYSICS_TYPES_HH

#include "core/core.hh"
#include "math/bbox.hh"

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

    struct aabb {
        explicit aabb(const float x, const float y, const float z,
                const float vx, const float vy, const float vz,
                const geometry::bbox<float>& bbox) noexcept
            : left{x + bbox.min_u}, back{y - kYCollisionDistance},
              right{x + bbox.max_u}, front{y + kYCollisionDistance},
              top{z + bbox.height()}, bottom{z}, vx{vx}, vy{vy}, vz{vz},
              bbox{bbox}
        {
        }

        float left, back, right, front, top, bottom, vx, vy, vz;
        geometry::bbox<float> bbox;
    };

    struct sweep_result {
        float time{1.0f};
        float normal_x{0.0f};
        float normal_y{0.0f};
        float normal_z{0.0f};
    };

} // namespace sc::physics

SC_ENABLE_ENUM_BITWISE_OPS(sc::physics::type)

#endif // SC_SIMULATION_PHYSICS_TYPES_HH
