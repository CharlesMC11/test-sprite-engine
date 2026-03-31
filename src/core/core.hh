/**
 * Source of truth for type aliases, constants, and enums.
 */
#ifndef SC_CORE_CORE_HH
#define SC_CORE_CORE_HH

#ifdef __METAL_VERSION__

#define SC_CONSTEXPR constant constexpr
#define SC_SIZE_T uint64_t

#else

#define SC_CONSTEXPR constexpr
#define SC_SIZE_T std::size_t

#include <cstdint>
#include <numeric>
#include <type_traits>

#endif

namespace sc::core {

    using index_t = uint32_t;
    static SC_CONSTEXPR auto kInvalidIndex{static_cast<index_t>(-1)};

    using physics_t = uint8_t;

    static SC_CONSTEXPR SC_SIZE_T kNeonAlignment{16UL};
    static SC_CONSTEXPR SC_SIZE_T kCacheAlignment{128UL};

#ifndef __METAL_VERSION__

    /**
     * Requirements for types to be safe to direct memory mapping.
     *
     * Type must be 16-byte aligned and follow Standard Layout to ensure the
     * CPU and GPU interpret the raw bytes identically.
     */
    template<typename T>
    concept mappable = alignof(T) % kNeonAlignment == 0UZ &&
            std::is_standard_layout_v<T> && !std::is_polymorphic_v<T>;

    template<typename T>
    constexpr bool enable_bitwise_ops_v = false;

    template<typename T>
    concept bitwise_enum =
            std::is_enum_v<T> && sc::core::enable_bitwise_ops_v<T>;

    template<bitwise_enum T>
    [[nodiscard]] constexpr bool any(const T val) noexcept
    {
        return static_cast<std::underlying_type_t<T>>(val) != 0;
    }

    using float_limits = std::numeric_limits<float>;

    static constexpr float kEpsilon{float_limits::epsilon()};
    static constexpr float kInfinity{float_limits::infinity()};

#endif // __METAL_VERSION__

} // namespace sc::core

#ifndef __METAL_VERSION__

template<sc::core::bitwise_enum T>
[[nodiscard]] constexpr T operator|(const T lhs, const T rhs) noexcept
{
    using U = std::underlying_type_t<T>;
    return static_cast<T>(static_cast<U>(lhs) | static_cast<U>(rhs));
}

template<sc::core::bitwise_enum T>
[[nodiscard]] constexpr T operator&(const T lhs, const T rhs) noexcept
{
    using U = std::underlying_type_t<T>;
    return static_cast<T>(static_cast<U>(lhs) & static_cast<U>(rhs));
}

template<sc::core::bitwise_enum T>
[[nodiscard]] constexpr T operator^(const T lhs, const T rhs) noexcept
{
    using U = std::underlying_type_t<T>;
    return static_cast<T>(static_cast<U>(lhs) ^ static_cast<U>(rhs));
}

template<sc::core::bitwise_enum T>
[[nodiscard]] constexpr T operator~(const T val) noexcept
{
    using U = std::underlying_type_t<T>;
    return static_cast<T>(~static_cast<U>(val));
}

template<sc::core::bitwise_enum T>
constexpr T& operator|=(T& lhs, const T rhs) noexcept
{
    return lhs = lhs | rhs;
}

template<sc::core::bitwise_enum T>
constexpr T& operator&=(T& lhs, const T rhs) noexcept
{
    return lhs = lhs & rhs;
}

template<sc::core::bitwise_enum T>
constexpr T& operator^=(T& lhs, const T rhs) noexcept
{
    return lhs = lhs ^ rhs;
}

#define SC_ENABLE_ENUM_BITWISE_OPS(EnumType)                                   \
    template<>                                                                 \
    constexpr bool sc::core::enable_bitwise_ops_v<EnumType> = true;

#endif // __METAL_VERSION__

#endif // SC_CORE_CORE_HH
