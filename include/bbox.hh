#pragma once

namespace sc::geometry {

    template<typename T>
    struct bbox final {

        constexpr T width() const noexcept { return right - left; }
        constexpr T height() const noexcept { return bottom - top; }

        T left, top, right, bottom;
    };

} // namespace sc::geometry
