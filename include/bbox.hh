/**
 * @file bbox.hh
 */
#pragma once

namespace sc::geometry {

    /**
     * @struct bbox
     * @brief A bounding box.
     * @tparam T Integer or float.
     *
     * This struct is compatible with both C++ and Metal.
     */
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
