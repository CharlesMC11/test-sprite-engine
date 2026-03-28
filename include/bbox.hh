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
        template<typename U>
        [[nodiscard]] explicit constexpr operator bbox<U>() const;
        [[nodiscard]] constexpr T width() const;
        [[nodiscard]] constexpr T height() const;

        T min_u, min_v, max_u, max_v;
    };

    template<typename T>
    template<typename U>
    [[nodiscard]] constexpr bbox<T>::operator bbox<U>() const
    {
        return {static_cast<U>(min_u), static_cast<U>(min_v),
                static_cast<U>(max_u), static_cast<U>(max_v)};
    }

    template<typename T>
    [[nodiscard]] constexpr T bbox<T>::width() const
    {
        return max_u - min_u;
    }

    template<typename T>
    [[nodiscard]] constexpr T bbox<T>::height() const
    {
        return max_v - min_v;
    }

} // namespace sc::geometry
