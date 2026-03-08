#pragma once

#include <cstdint>

#include "atlas_indices.h"
#include "sprite.h"

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
                atlas_indices i) const noexcept;

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
            const atlas_indices i) const noexcept
    {
        return data_[i];
    }

} // namespace sc
