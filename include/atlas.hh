/**
 * @file atlas.hh
 * @brief
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "atlas_index.hh"
#include "core.hh"
#include "sprite.hh"

namespace sc::sprites {

    /**
     * @struct atlas
     * @brief A contiguous collection of sprites.
     *
     * This class is designed to live within an `sc::sys::file_mapping`. It uses
     * a flexible array member (`sprites`) to provide indexed access to sprites
     * loaded directly from an `.atlas` file.
     */
    struct alignas(core::kAlignment) atlas final {
        uint64_t magic;
        uint64_t count;
        sprite data[];

        // Delete constructors because the atlas is mapped, not
        // instantiated.
        atlas() = delete;
        ~atlas() = delete;
        atlas(const atlas&) = delete;
        atlas(atlas&&) = delete;
        atlas& operator=(const atlas&) = delete;
        atlas& operator=(atlas&&) = delete;

        [[nodiscard]] constexpr const sprite& operator[](
                core::index_t i) const noexcept;

        [[nodiscard]] constexpr const sprite& operator[](
                atlas_index i) const noexcept;

        [[nodiscard]] constexpr bool is_valid(
                std::size_t expected_size) const noexcept;
    };

    [[nodiscard]] constexpr const sprite& atlas::operator[](
            const core::index_t i) const noexcept
    {
        return data[i];
    }

    [[nodiscard]] constexpr const sprite& atlas::operator[](
            const atlas_index i) const noexcept
    {
        return (*this)[static_cast<core::index_t>(i)];
    }

    [[nodiscard]] constexpr bool atlas::is_valid(
            const std::size_t expected_size) const noexcept
    {
        return magic == kAtlasMagicBytes &&
                sizeof(atlas) + count * sizeof(sprite) == expected_size;
    }

} // namespace sc::sprites
