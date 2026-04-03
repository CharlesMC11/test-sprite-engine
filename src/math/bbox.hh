#ifndef SC_MATH_BBOX_HH
#define SC_MATH_BBOX_HH

#ifndef __METAL_VERSION__

#include <cstdint>

#endif

namespace sc::geometry {

    /**
     * A bounding box.
     *
     * @tparam T
     * Integer or float.
     *
     * This struct is compatible with both C++ and Metal.
     */
    template<typename T = uint8_t>
    struct alignas(sizeof(T) * 4UL) bbox final {
        // Operators

        template<typename U>
        [[nodiscard]] explicit constexpr operator bbox<U>() const;

        // Accessors

        [[nodiscard]] constexpr T width() const;
        [[nodiscard]] constexpr T height() const;

        // Attributes

        T u_min{0};
        T u_max{0};

        T v_min{0};
        T v_max{0};
    };

    // Operators

    template<typename T>
    template<typename U>
    [[nodiscard]] constexpr bbox<T>::operator bbox<U>() const
    {
        return {static_cast<U>(u_min), static_cast<U>(u_max),
                static_cast<U>(v_min), static_cast<U>(v_max)};
    }

    // Accessors

    template<typename T>
    [[nodiscard]] constexpr T bbox<T>::width() const
    {
        return u_max - u_min;
    }

    template<typename T>
    [[nodiscard]] constexpr T bbox<T>::height() const
    {
        return v_max - v_min;
    }

} // namespace sc::geometry

#endif // SC_MATH_BBOX_HH
