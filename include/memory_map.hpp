#pragma once

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string_view>
#include <type_traits>

namespace sc {

    template<typename T>
    concept mappable = alignof(T) == 16 && std::is_trivially_copyable_v<T> &&
            std::is_standard_layout_v<T>;

    template<mappable T>
    class memory_map final {
    public:
        [[nodiscard]] explicit memory_map(const char path[]) noexcept;
        ~memory_map() noexcept;

        memory_map(const memory_map&) = delete;
        memory_map& operator=(const memory_map&) = delete;

        [[nodiscard]] constexpr const T* operator->() const noexcept;
        [[nodiscard]] constexpr const T& operator*() const noexcept;
        [[nodiscard]] explicit constexpr operator bool() const noexcept;

        [[nodiscard]] constexpr const T* data() const noexcept;

    private:
        const T* buffer_ = nullptr;
    };

    template<mappable T>
    memory_map<T>::memory_map(const char path[]) noexcept
    {
        const int fd{open(path, O_RDONLY)};
        if (fd < 0) [[unlikely]]
            return;

        struct stat st;
        if (fstat(fd, &st) < 0 || st.st_size != sizeof(T)) [[unlikely]] {
            close(fd);
            return;
        }

        const void* result =
                mmap(nullptr, sizeof(T), PROT_READ, MAP_SHARED, fd, 0);
        if (result == MAP_FAILED) [[unlikely]] {
            close(fd);
            return;
        }

        buffer_ = static_cast<const T*>(result);
        close(fd);
    }

    template<mappable T>
    memory_map<T>::~memory_map() noexcept
    {
        if (buffer_)
            munmap(const_cast<T*>(buffer_), sizeof(T));
    }

    template<mappable T>
    [[nodiscard]] constexpr const T* memory_map<T>::operator->() const noexcept
    {
        return buffer_;
    }

    template<mappable T>
    [[nodiscard]] constexpr const T& memory_map<T>::operator*() const noexcept
    {
        return *buffer_;
    }

    template<mappable T>
    [[nodiscard]] constexpr memory_map<T>::operator bool() const noexcept
    {
        return buffer_ != nullptr;
    }

    template<mappable T>
    [[nodiscard]] constexpr const T* memory_map<T>::data() const noexcept
    {
        return buffer_;
    }

} // namespace sc
