#pragma once

namespace sc::geometry {

    template<typename T>
    struct bbox final {

        [[nodiscard]] constexpr T width() const;
        [[nodiscard]] constexpr T height() const;

        T left, top, right, bottom;
    };

    template<typename T>
    [[nodiscard]] constexpr T bbox<T>::width() const
    {
        return right - left;
    }

    template<typename T>
    [[nodiscard]] constexpr T bbox<T>::height() const
    {
        return bottom - top;
    }

} // namespace sc::geometry
