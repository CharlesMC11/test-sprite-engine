/**
 * @file atlas.hh
 * @brief
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "core.hh"
#include "palette_index.hh"
#include "sprite.hh"
#include "sprite_index.hh"

namespace sc::sprites {

    static constexpr core::atlas_magic_t kAtlasMagicBytes{0x3376205441204353};

    /**
     * @struct atlas
     * @brief A contiguous collection of sprites.
     *
     * This class is designed to live within an `sc::core::file_mapping`. It
     * uses a flexible array member `data` to provide indexed access to sprites
     * loaded directly from an `.atlas` file.
     */
    struct alignas(core::kCacheAlignment) atlas final {

        // Delete constructors because the atlas is mapped, not instantiated.
        atlas() = delete;
        atlas(const atlas&) = delete;
        atlas(atlas&&) = delete;

        ~atlas() = delete;

        atlas& operator=(const atlas&) = delete;
        atlas& operator=(atlas&&) = delete;

        [[nodiscard]] constexpr const std::byte* data() const noexcept;

        [[nodiscard]] constexpr std::span<const core::packed_color_t*>
        palettes() const noexcept;

        [[nodiscard]] constexpr std::span<const sprite32>
        sprites() const noexcept;

        [[nodiscard]] constexpr core::packed_color_t* operator[](
                palette_index i) const noexcept;

        [[nodiscard]] constexpr const sprite32& operator[](
                sprite_index i) const noexcept;

        [[nodiscard]] static constexpr bool validate(
                const void* ptr, std::size_t mapped_size) noexcept;

        struct alignas(core::kNeonAlignment) metadata final {
            core::atlas_magic_t magic;
            std::uint32_t palette_count;
            std::uint32_t sprite_count;
        } metadata;
    };

    [[nodiscard]] constexpr const std::byte* atlas::data() const noexcept
    {
        return reinterpret_cast<const std::byte*>(&metadata + 1);
    }

    [[nodiscard]] constexpr const sprite& atlas::operator[](
            const atlas_index i) const noexcept
    {
        return (*this)[static_cast<core::atlas_index_t>(i)];
    }

    [[nodiscard]] constexpr bool atlas::validate(
            const void* ptr, const std::size_t mapped_size) noexcept
    {
        if (!ptr || mapped_size < sizeof(atlas))
            return false;

        const struct metadata metadata{
                static_cast<const atlas*>(ptr)->metadata};
        if (metadata.magic != kAtlasMagicBytes)
            return false;

        const std::size_t expected_size{sizeof(metadata) +
                metadata.palette_count *
                        sizeof(core::packed_color_t[kMaxPaletteSize]) +
                metadata.sprite_count * sizeof(sprite32)};
        return mapped_size >= expected_size;
    }

} // namespace sc::sprites
