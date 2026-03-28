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

        T min_u, min_v, max_u, max_v;
    };

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
