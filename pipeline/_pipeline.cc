#include <nanobind/nanobind.h>

#include "assets/atlas.hh"
#include "assets/sprite.hh"
#include "core/core.hh"
#include "graphics/graphics_types.hh"
#include "math/utils.hh"
#include "simulation/physics_types.hh"

namespace nb = nanobind;

NB_MODULE(_pipeline, m)
{
    m.doc() = "Pipeline Bridge Library";

    m.attr("MAX_2_BIT") = sc::core::k2BitMax;
    m.attr("MAX_5_BIT") = sc::core::k5BitMax;
    m.attr("MAX_6_BIT") = sc::core::k6BitMax;

    m.attr("NEON_ALIGNMENT") = sc::core::kNeonAlignment;
    m.attr("CACHE_ALIGNMENT") = sc::core::kCacheAlignment;

    m.attr("MAX_PALETTE_SIZE") = sc::graphics::kMaxPaletteSize;

    m.attr("PACKED_COLOR_SIZE_BYTES") = sizeof(sc::graphics::packed_color_t);

    m.attr("PALETTE_SIZE_BYTES") = sc::graphics::kMaxPaletteSize *
        sizeof(sc::graphics::packed_color_t);

    nb::enum_<sc::graphics::color_encoding>(
            m, "ColorEncoding", "Distribution of color bits across a 2-byte packed integer.", nb::is_arithmetic())
        .value("NEUTRAL", sc::graphics::color_encoding::neutral, "R5G6B5")
        .value("WARM", sc::graphics::color_encoding::warm, "R6G5B5")
        .value("COOL", sc::graphics::color_encoding::cool, "R5G5B6");

    nb::enum_<sc::physics::type>(
            m, "PhysicsType", "The laws of physics an entity obeys.", nb::is_arithmetic(), nb::is_flag())
        .value("UNDEFINED", sc::physics::type::UNDEFINED)
        .value("NONE", sc::physics::type::NONE)
        .value("ACTOR", sc::physics::type::ACTOR)
        .value("STATIC", sc::physics::type::STATIC)
        .value("SENSOR", sc::physics::type::SENSOR)
        .value("PROJECTILE", sc::physics::type::PROJECTILE);
}
