#ifndef SC_CORE_MEMORY_HH
#define SC_CORE_MEMORY_HH

#include <Metal/Metal.hpp>

#include <cstring>

#include "core/core.hh"
#include "math/utils.hh"

namespace sc::mem {

    /**
     * A dynamic array meant to contain at least 2 subarrays.
     *
     * @tparam T
     * The type of the contained elements.
     *
     * @tparam N
     * The number of subarrays.
     */
    template<typename T, std::size_t N = 2UZ>
    struct channel_pool final {
        // Constructors

        [[nodiscard]] constexpr channel_pool() = default;

        channel_pool(const channel_pool&) = delete;
        channel_pool& operator=(const channel_pool&) = delete;

        channel_pool(channel_pool&&) = delete;
        channel_pool& operator=(channel_pool&&) = delete;

        ~channel_pool() = default;

        // Operators

        /**
         * Get the subarray
         *
         * @param i
         * The index.
         *
         * @return
         * The subarray.
         */
        [[nodiscard]] constexpr auto operator[](std::size_t i) const noexcept
                -> T* __restrict;

        // Mutators

        /**
         * Increate the capacity of each subarray.
         *
         * @param device
         * The metal device.
         *
         * @param new_capacity
         * The new capacity size.
         */
        constexpr void grow(MTL::Device* device, std::size_t new_capacity);

        // Accessors

        /**
         * Calculate the subarray’s offset from the base pointer.
         *
         * @param i
         * The subarray’s index.
         *
         * @return
         * The offset.
         */
        [[nodiscard]] constexpr auto subarray_offset(
                std::size_t i) const noexcept -> std::size_t;

        // Attributes

        NS::SharedPtr<MTL::Buffer> buffer{nullptr};
        std::size_t count{0UZ};
        std::size_t capacity{0UZ};
    };

    // Operators

    template<typename T, std::size_t N>
    [[nodiscard]] constexpr auto channel_pool<T, N>::operator[](
            const std::size_t i) const noexcept -> T* __restrict
    {
        if (!buffer) [[unlikely]]
            return nullptr;

        auto* base{static_cast<std::byte*>(buffer->contents())};
        return reinterpret_cast<T*>(base + subarray_offset(i));
    }

    // Mutators

    template<typename T, std::size_t N>
    constexpr void channel_pool<T, N>::grow(
            MTL::Device* device, const std::size_t new_capacity)
    {
        const std::size_t aligned_new_capacity{
                math::align_up(new_capacity, core::kCacheAlignment)};
        if (aligned_new_capacity <= capacity) [[unlikely]]
            return;

        const std::size_t new_total_bytes{sizeof(T) * aligned_new_capacity * N};
        const NS::SharedPtr new_buffer{NS::TransferPtr(device->newBuffer(
                new_total_bytes, MTL::ResourceStorageModeShared))};

        if (buffer) [[likely]] {
            const auto* src{static_cast<const std::byte*>(buffer->contents())};
            auto* dst{static_cast<std::byte*>(new_buffer->contents())};

            for (std::size_t i{0UZ}; i < N; ++i) {
                std::memcpy(dst + sizeof(T) * aligned_new_capacity * i,
                        src + subarray_offset(i), sizeof(T) * count);
            }
        }

        buffer = new_buffer;
        capacity = aligned_new_capacity;
    }

    // Accessors

    template<typename T, std::size_t N>
    [[nodiscard]] constexpr auto channel_pool<T, N>::subarray_offset(
            const std::size_t i) const noexcept -> std::size_t
    {
        return sizeof(T) * capacity * i;
    }

} // namespace sc::mem

#endif // SC_CORE_MEMORY_HH
