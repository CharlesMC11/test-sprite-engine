#ifndef SC_CORE_INPUT_HH
#define SC_CORE_INPUT_HH

#include <cstdint>

#include "core/core.hh"

namespace sc::input {

    enum class mask : std::uint8_t {
        none = 0U,
        up = 1U,
        down = 1U << 1U,
        left = 1U << 2U,
        right = 1U << 3U,
        jump = 1U << 4U,
    };

} // namespace sc::input

SC_ENABLE_ENUM_BITWISE_OPS(sc::input::mask)

#endif // SC_CORE_INPUT_HH
