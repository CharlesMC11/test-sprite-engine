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

        // Delete constructors because the atlas is mapped, not instantiated.
        atlas() = delete;
        atlas(const atlas&) = delete;
        atlas(atlas&&) = delete;

        ~atlas() = delete;

        atlas& operator=(const atlas&) = delete;
        atlas& operator=(atlas&&) = delete;

        [[nodiscard]] constexpr const sprite& operator[](
                core::index_t i) const noexcept;

        [[nodiscard]] constexpr const sprite& operator[](
                atlas_index i) const noexcept;

        [[nodiscard]] static constexpr bool validate(
                const void* ptr, std::size_t mapped_size) noexcept;

        uint64_t magic;
        uint64_t count;
        sprite data[];
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

    [[nodiscard]] constexpr bool atlas::validate(
            const void* ptr, const std::size_t mapped_size) noexcept
    {
        if (!ptr || mapped_size < sizeof(atlas))
            return false;

        const auto* header{static_cast<const atlas*>(ptr)};
        if (header->magic != kAtlasMagicBytes)
            return false;

        const std::size_t expected_size{
                sizeof(atlas) + header->count * sizeof(sprite)};
        return mapped_size >= expected_size;
    }

} // namespace sc::sprites
