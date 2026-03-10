/**
 * @file mapped_asset.hpp
 * @brief RAII wrapper for POSIX memory-mapped files.
 */
#pragma once

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstddef>
#include <type_traits>

namespace sc {

    /**
     * @concept mappable
     * @brief Requirements for types to be safe to direct memory mapping.
     *
     * Type must be 16-byte aligned and follow Standard Layout to ensure the CPU
     * and GPU interpret the raw bytes identically.
     */
    template<typename T>
    concept mappable = alignof(T) == 16 && std::is_standard_layout_v<T> &&
            !std::is_polymorphic_v<T>;

    /**
     * @class mapped_asset
     * @brief Manages `mmap`/`munmap` lifecycle for a specific file path.
     * @tparam T The type to cast the mapped memory to.
     */
    template<mappable T>
    class mapped_asset final {
    public:
        [[nodiscard]] explicit mapped_asset(const char path[]) noexcept;
        ~mapped_asset() noexcept;

        mapped_asset(const mapped_asset&) = delete;
        mapped_asset& operator=(const mapped_asset&) = delete;

        [[nodiscard]] constexpr const T* operator->() const noexcept;
        [[nodiscard]] constexpr const T& operator*() const noexcept;
        [[nodiscard]] explicit constexpr operator bool() const noexcept;

        [[nodiscard]] constexpr std::size_t size() const noexcept;
        [[nodiscard]] constexpr const T* data() const noexcept;

    private:
        std::size_t size_{0};
        const T* buffer_ = nullptr;
    };

    template<mappable T>
    mapped_asset<T>::mapped_asset(const char path[]) noexcept
    {
        const int fd{open(path, O_RDONLY)};
        if (fd < 0) [[unlikely]]
            return;

        struct stat st;
        if (fstat(fd, &st) < 0) [[unlikely]] {
            close(fd);
            return;
        }
        size_ = static_cast<std::size_t>(st.st_size);

        const void* result = mmap(nullptr, size_, PROT_READ, MAP_SHARED, fd, 0);
        if (result == MAP_FAILED) [[unlikely]] {
            close(fd);
            return;
        }

        buffer_ = static_cast<const T*>(result);
        close(fd);
    }

    template<mappable T>
    mapped_asset<T>::~mapped_asset() noexcept
    {
        if (buffer_) [[likely]]
            munmap(const_cast<T*>(buffer_), size_);
    }

    template<mappable T>
    [[nodiscard]] constexpr const T*
    mapped_asset<T>::operator->() const noexcept
    {
        return buffer_;
    }

    template<mappable T>
    [[nodiscard]] constexpr const T& mapped_asset<T>::operator*() const noexcept
    {
        return *buffer_;
    }

    template<mappable T>
    [[nodiscard]] constexpr mapped_asset<T>::operator bool() const noexcept
    {
        return buffer_ != nullptr;
    }

    template<mappable T>
    [[nodiscard]] constexpr std::size_t mapped_asset<T>::size() const noexcept
    {
        return size_;
    }

    template<mappable T>
    [[nodiscard]] constexpr const T* mapped_asset<T>::data() const noexcept
    {
        return buffer_;
    }

} // namespace sc
