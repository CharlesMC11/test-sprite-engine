#pragma once

#include <cstdint>

#include "atlas_index.hpp"
#include "sprite.hpp"

namespace sc {

    class alignas(16) atlas final {
    public:
        atlas() = delete;
        ~atlas() = delete;
        atlas(const atlas&) = delete;
        atlas(atlas&&) = delete;
        atlas& operator=(const atlas&) = delete;
        atlas& operator=(atlas&&) = delete;

        [[nodiscard]] constexpr std::uint64_t size() const noexcept;

        [[nodiscard]] constexpr const sprite& operator[](
                std::size_t i) const noexcept;

        [[nodiscard]] constexpr const sprite& operator[](
                atlas_index i) const noexcept;

    private:
        char magic_[8];
        uint64_t count_;
        sprite data_[];
    };

    [[nodiscard]] constexpr std::uint64_t atlas::size() const noexcept
    {
        return count_;
    }

    [[nodiscard]] constexpr const sprite& atlas::operator[](
            const std::size_t i) const noexcept
    {
        return data_[i];
    }

    [[nodiscard]] constexpr const sprite& atlas::operator[](
            const atlas_index i) const noexcept
    {
        return (*this)[static_cast<std::size_t>(i)];
    }

} // namespace sc
