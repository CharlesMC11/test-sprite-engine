#ifndef SC_SIMULATION_PHYSICS_HH
#define SC_SIMULATION_PHYSICS_HH

#include "assets/atlas.hh"
#include "registry/entity_registry.hh"
#include "spatial_grid.hh"

namespace sc::physics {

    void resolve_entity_collisions(entity_registry& registry,
            const spatial_grid& grid, const assets::atlas& atlas, float dt);

} // namespace sc::physics

#endif // SC_SIMULATION_PHYSICS_HH
