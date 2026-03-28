#pragma once

#include <cstdlib>
#include <memory>

#include "Metal/MTL4PipelineDataSetSerializer.hpp"
#include "core.hh"

namespace sc::mem {

    struct c_deleter {
        void operator()(void* ptr) const { std::free(ptr); }
    };

    template<typename T, std::size_t N>
    struct soa_block {

        constexpr T* operator[](std::size_t i) const noexcept;

        constexpr void grow(std::size_t new_capacity);

        std::unique_ptr<T[], c_deleter> data{nullptr};
        std::size_t count{0u};
        std::size_t capacity{0u};
    };

    template<typename T, std::size_t N>
    constexpr T* soa_block<T, N>::operator[](const std::size_t i) const noexcept
    {
        return data.get() + capacity * i;
    }

    template<typename T, std::size_t N>
    constexpr void soa_block<T, N>::grow(const std::size_t new_capacity)
    {
        const std::size_t aligned_new_capacity{
                new_capacity + core::kCacheAlignment - 1u &
                ~(core::kCacheAlignment - 1u)};
        if (aligned_new_capacity <= capacity)
            return;

        T* old_buffer{data.get()};
        auto* new_buffer{static_cast<T*>(std::realloc(
                old_buffer, sizeof(T) * aligned_new_capacity * N))};
        if (!new_buffer) {
            return;
        }

        for (std::ptrdiff_t i{N - 1}; i >= 0; --i) {
            const T* src{old_buffer + i * capacity};
            if (T* dst{new_buffer + i * aligned_new_capacity}; src != dst)
                std::memmove(dst, src, sizeof(T) * count);
        }

        if (old_buffer != new_buffer) {
            data.release();
            data.reset(new_buffer);
        }
        capacity = aligned_new_capacity;
    }

} // namespace sc::mem
