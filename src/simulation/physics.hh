#ifndef SC_SIMULATION_PHYSICS_HH
#define SC_SIMULATION_PHYSICS_HH

#include "assets/atlas.hh"
#include "registry/entity_registry.hh"

namespace sc::physics {

    void resolve_entity_collisions(
            const assets::atlas& atlas, entity_registry& registry, float dt);

} // namespace sc::physics

#endif // SC_SIMULATION_PHYSICS_HH
