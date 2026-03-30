#ifndef SC_CORE_INPUT_HH
#define SC_CORE_INPUT_HH

#include <cstdint>

#include "core/core.hh"

namespace sc::input {

    enum class mask : std::uint8_t {
        NONE = 0u,
        UP = 1u,
        DOWN = 1u << 1u,
        LEFT = 1u << 2u,
        RIGHT = 1u << 3u,
    };

} // namespace sc::input

SC_ENABLE_ENUM_BITWISE_OPS(sc::input::mask)

#endif // SC_CORE_INPUT_HH
