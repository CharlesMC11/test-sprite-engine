#ifndef SC_CORE_INPUT_HH
#define SC_CORE_INPUT_HH

#include <cstdint>

#include "core/core.hh"

namespace sc::input {

    enum class mask : std::uint8_t {
        NONE = 0U,
        UP = 1U,
        DOWN = 1U << 1,
        LEFT = 1U << 2,
        RIGHT = 1U << 3,
    };

} // namespace sc::input

SC_ENABLE_ENUM_BITWISE_OPS(sc::input::mask)

#endif // SC_CORE_INPUT_HH
