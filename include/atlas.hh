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

    static constexpr std::uint64_t kAtlasMagicBytes{0x3376205441204353};

    /**
     * @struct atlas
     * @brief A contiguous collection of sprites.
     *
     * This class is designed to live within an `sc::core::file_mapping`.
     */
    struct alignas(core::kCacheAlignment) atlas final {
        [[nodiscard]] static constexpr bool validate(
                const void* ptr, std::size_t mapped_size) noexcept;

        // Delete constructors because the atlas is mapped, not instantiated.
        atlas() = delete;
        atlas(const atlas&) = delete;
        atlas(atlas&&) = delete;

        ~atlas() = delete;

        atlas& operator=(const atlas&) = delete;
        atlas& operator=(atlas&&) = delete;

        [[nodiscard]] constexpr auto operator[](palette_index i) const noexcept
                -> const palette&;

        [[nodiscard]] constexpr auto operator[](sprite_index i) const noexcept
                -> const sprite32x32&;

        [[nodiscard]] constexpr auto data() const noexcept -> const std::byte*;

        [[nodiscard]] constexpr auto palettes() const noexcept
                -> std::span<const palette>;

        [[nodiscard]] constexpr auto sprites() const noexcept
                -> std::span<const sprite32x32>;

        struct alignas(core::kNeonAlignment) metadata final {
            std::uint64_t magic;
            std::uint32_t palette_count;
            std::uint32_t sprite_count;
        } metadata;
    };

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
                metadata.palette_count * sizeof(palette) +
                metadata.sprite_count * sizeof(sprite32x32)};
        return mapped_size >= expected_size;
    }

    [[nodiscard]] constexpr auto atlas::operator[](
            const palette_index i) const noexcept -> const palette&
    {
        return palettes()[static_cast<std::size_t>(i)];
    }

    [[nodiscard]] constexpr auto atlas::operator[](
            const sprite_index i) const noexcept -> const sprite32x32&
    {
        return sprites()[static_cast<std::size_t>(i)];
    }

    [[nodiscard]] constexpr auto atlas::data() const noexcept
            -> const std::byte*
    {
        return reinterpret_cast<const std::byte*>(&metadata + 1);
    }

    [[nodiscard]] constexpr auto atlas::palettes() const noexcept
            -> std::span<const palette>
    {
        return {reinterpret_cast<const palette*>(data()),
                sizeof(palette) * metadata.palette_count};
    }

    [[nodiscard]] constexpr auto atlas::sprites() const noexcept
            -> std::span<const sprite32x32>
    {
        return {reinterpret_cast<const sprite32x32*>(
                        data() + sizeof(palette) * metadata.palette_count),
                sizeof(sprite32x32) * metadata.sprite_count};
    }

} // namespace sc::sprites
