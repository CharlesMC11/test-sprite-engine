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
        UNDEFINED = 0u,
        NONE = 1u,
        ACTOR = 1u << 1u,
        STATIC = 1u << 2u,
        SENSOR = 1u << 3u,
        PROJECTILE = 1u << 4u,
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
