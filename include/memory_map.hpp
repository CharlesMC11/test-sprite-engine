#pragma once

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string_view>

namespace sc {

    template<typename T>
    class memory_map {
    public:
        explicit memory_map(const char path[]);
        ~memory_map();

        memory_map(const memory_map&) = delete;
        memory_map& operator=(const memory_map&) = delete;

        const T* operator->() const noexcept { return buffer_; }
        const T& operator*() const noexcept { return *buffer_; }

        explicit operator bool() const noexcept;

        const T* data() const noexcept { return buffer_; }

    private:
        const T* buffer_ = nullptr;
    };

    template<typename T>
    memory_map<T>::memory_map(const char path[])
    {
        const int fd{open(path, O_RDONLY)};
        if (fd < 0)
            return;

        struct stat st;
        if (fstat(fd, &st) < 0 || st.st_size != sizeof(T)) {
            close(fd);
            return;
        }

        buffer_ = static_cast<T*>(
                mmap(nullptr, sizeof(T), PROT_READ, MAP_SHARED, fd, 0));
        if (buffer_ == MAP_FAILED) {
            close(fd);
            return;
        }

        close(fd);
    }

    template<typename T>
    memory_map<T>::~memory_map()
    {
        if (*this)
            munmap(const_cast<T*>(buffer_), sizeof(T));
    }

    template<typename T>
    memory_map<T>::operator bool() const noexcept
    {
        return buffer_ != nullptr && buffer_ != MAP_FAILED;
    }

} // namespace sc
