#pragma once

#include "core.hh"

namespace sc::input {

    enum class mask : core::input_mask_t {
        UP = 0x01,
        DOWN = 0x02,
        LEFT = 0x04,
        RIGHT = 0x08,
    };

    constexpr core::input_mask_t operator&(const core::input_mask_t a, mask b)
    {
        return a & static_cast<core::input_mask_t>(b);
    }

    constexpr core::input_mask_t& operator&=(core::input_mask_t& a, mask b)
    {
        a &= static_cast<core::input_mask_t>(b);
        return a;
    }

    constexpr core::input_mask_t& operator|=(core::input_mask_t& a, mask b)
    {
        a |= static_cast<core::input_mask_t>(b);
        return a;
    }

    constexpr core::input_mask_t operator~(const mask& m)
    {
        return ~static_cast<core::input_mask_t>(m);
    }

} // namespace sc::input
