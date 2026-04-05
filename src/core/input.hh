#ifndef SC_CORE_INPUT_HH
#define SC_CORE_INPUT_HH

#include <cstdint>

#include "core/core.hh"

namespace sc::input {

    enum class mask : std::uint8_t {
        none = 0U,
        up = 1U,
        down = 1U << 1,
        left = 1U << 2,
        right = 1U << 3,
        jump = 1U << 4,
    };

} // namespace sc::input

SC_ENABLE_ENUM_BITWISE_OPS(sc::input::mask)

#endif // SC_CORE_INPUT_HH
