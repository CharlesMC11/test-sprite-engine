#ifndef SC_SIMULATION_WORLD_HH
#define SC_SIMULATION_WORLD_HH

#include <Metal/Metal.hpp>

#include "assets/atlas.hh"
#include "core/input.hh"
#include "core/mapped_view.hh"
#include "graphics/metal_bridge.hh"
#include "registry/entity_registry.hh"
#include "simulation/spatial_grid.hh"

namespace sc {

    class world final {
    public:
        // Constructors

        [[nodiscard]] explicit world(MTL::Device* device);

        // Public methods

        void update(input::mask input, const MTL::Drawable* drawable);

    private:
        physics::spatial_grid grid_;
        render::metal_bridge bridge_;
        entity_registry registry_;
        core::mapped_view<assets::atlas> atlas_;
        float ftime_accumulator_{0.0f};
    };

} // namespace sc

#endif // SC_SIMULATION_WORLD_HH
