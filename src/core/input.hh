#ifndef SC_CORE_INPUT_HH
#define SC_CORE_INPUT_HH

#include "core/core.hh"

namespace sc::input {

    enum class mask : core::input_mask {
        UP = 0x01,
        DOWN = 0x02,
        LEFT = 0x04,
        RIGHT = 0x08,
    };

    constexpr core::input_mask operator&(const core::input_mask a, mask b)
    {
        return a & static_cast<core::input_mask>(b);
    }

    constexpr core::input_mask& operator&=(core::input_mask& a, mask b)
    {
        a &= static_cast<core::input_mask>(b);
        return a;
    }

    constexpr core::input_mask& operator|=(core::input_mask& a, mask b)
    {
        a |= static_cast<core::input_mask>(b);
        return a;
    }

    constexpr core::input_mask operator~(const mask& m)
    {
        return ~static_cast<core::input_mask>(m);
    }

} // namespace sc::input

#endif // SC_CORE_INPUT_HH
