/**
 * @file atlas.hpp
 * @brief
 */
#pragma once

#include <cstddef>
#include <cstdint>

#include "atlas_index.hpp"
#include "sprite.hpp"

namespace sc {

    /**
     * @class atlas
     * @brief A contiguous collection of sprites.
     *
     * This class is designed to live within an `sc::memory_map`. It uses a
     * flexible array member (`data_`) to provide indexed access to sprites
     * loaded directly from an `.atlas` file.
     */
    class alignas(16) atlas final {
    public:
        // Delete constructors because the atlas is mapped, not instantiated.
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
