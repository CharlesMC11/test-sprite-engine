#ifndef SC_MATH_UTILS_HH
#define SC_MATH_UTILS_HH

#include <cstddef>

namespace sc::math {

    /**
     * Aligns up a value to the specified power of 2.
     *
     * @param val
     * The value to align.
     *
     * @param alignment
     * The alignment to match.
     *
     * @return
     * The aligned value.
     */
    [[nodiscard]] constexpr std::size_t align_up(
            const std::size_t val, const std::size_t alignment) noexcept
    {
        return (val + alignment - 1UZ) & ~(alignment - 1UZ);
    }

} // namespace sc::math

#endif // SC_MATH_UTILS_HH
