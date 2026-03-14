/**
 * @file file_mapping.hh
 * @brief RAII wrapper for POSIX memory-mapped files.
 */
#pragma once

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstddef>
#include <type_traits>

#include "core.hh"

namespace sc::core {

    /**
     * @concept mappable
     * @brief Requirements for types to be safe to direct memory mapping.
     *
     * Type must be 16-byte aligned and follow Standard Layout to ensure the CPU
     * and GPU interpret the raw bytes identically.
     */
    template<typename T>
    concept mappable = alignof(T) == kAlignment &&
            std::is_standard_layout_v<T> && !std::is_polymorphic_v<T>;

    /**
     * @class file_mapping
     * @brief Manages `mmap`/`munmap` lifecycle for a specific file path.
     * @tparam T The type to cast the mapped memory to.
     */
    template<mappable T>
    class file_mapping final {
    public:
        [[nodiscard]] explicit constexpr file_mapping(
                const char path[]) noexcept;
        file_mapping(const file_mapping&) = delete;
        file_mapping(file_mapping&&) = delete;

        ~file_mapping() noexcept;

        file_mapping& operator=(const file_mapping&) = delete;
        file_mapping& operator=(file_mapping&&) = delete;

        [[nodiscard]] explicit constexpr operator bool() const noexcept;
        [[nodiscard]] constexpr const T* operator->() const noexcept;
        [[nodiscard]] constexpr const T& operator*() const noexcept;

        [[nodiscard]] constexpr const T* data() const noexcept;
        [[nodiscard]] constexpr std::size_t size() const noexcept;

    private:
        const T* buffer_ = nullptr;
        std::size_t size_{0};
    };

    template<mappable T>
    constexpr file_mapping<T>::file_mapping(const char path[]) noexcept
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
    file_mapping<T>::~file_mapping() noexcept
    {
        if (buffer_) [[likely]]
            munmap(const_cast<T*>(buffer_), size_);
    }

    template<mappable T>
    [[nodiscard]] constexpr file_mapping<T>::operator bool() const noexcept
    {
        return buffer_ != nullptr;
    }

    template<mappable T>
    [[nodiscard]] constexpr const T*
    file_mapping<T>::operator->() const noexcept
    {
        return buffer_;
    }

    template<mappable T>
    [[nodiscard]] constexpr const T& file_mapping<T>::operator*() const noexcept
    {
        return *buffer_;
    }

    template<mappable T>
    [[nodiscard]] constexpr const T* file_mapping<T>::data() const noexcept
    {
        return buffer_;
    }

    template<mappable T>
    [[nodiscard]] constexpr std::size_t file_mapping<T>::size() const noexcept
    {
        return size_;
    }

} // namespace sc::core
