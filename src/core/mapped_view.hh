#ifndef SC_CORE_MAPPED_VIEW_HH
#define SC_CORE_MAPPED_VIEW_HH

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstddef>

#include "core/core.hh"

namespace sc::core {

    /**
     * RAII wrapper for POSIX memory-mapped files.
     *
     * @tparam T
     * The type to cast the mapped memory to.
     */
    template<mappable T>
    class mapped_view final {
    public:
        // Constructors

        [[nodiscard]] explicit constexpr mapped_view(
                const char path[]) noexcept;

        mapped_view(const mapped_view&) = delete;
        mapped_view& operator=(const mapped_view&) = delete;

        mapped_view(mapped_view&&) = delete;
        mapped_view& operator=(mapped_view&&) = delete;

        ~mapped_view() noexcept;

        // Operators

        [[nodiscard]] explicit constexpr operator bool() const noexcept;
        [[nodiscard]] constexpr auto operator->() const noexcept
                -> const T* __restrict;

        // Accessors

        [[nodiscard]] constexpr auto data() const noexcept
                -> const T* __restrict;
        [[nodiscard]] constexpr std::size_t size() const noexcept;

    private:
        // Attributes

        const T* buffer_ = nullptr;
        std::size_t size_{0};
    };

    // Constructors

    template<mappable T>
    constexpr mapped_view<T>::mapped_view(const char path[]) noexcept
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
    mapped_view<T>::~mapped_view() noexcept
    {
        if (buffer_) [[likely]]
            munmap(const_cast<T*>(buffer_), size_);
    }

    // Operators

    template<mappable T>
    [[nodiscard]] constexpr mapped_view<T>::operator bool() const noexcept
    {
        return buffer_ != nullptr;
    }

    template<mappable T>
    [[nodiscard]] constexpr auto mapped_view<T>::operator->() const noexcept
            -> const T* __restrict
    {
        return buffer_;
    }

    // Accessors

    template<mappable T>
    [[nodiscard]] constexpr auto mapped_view<T>::data() const noexcept
            -> const T* __restrict
    {
        return buffer_;
    }

    template<mappable T>
    [[nodiscard]] constexpr std::size_t mapped_view<T>::size() const noexcept
    {
        return size_;
    }

} // namespace sc::core

#endif // SC_CORE_MAPPED_VIEW_HH
