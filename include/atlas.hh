/**
 * @file atlas.hh
 * @brief Core definitions and atlas data structures.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "core.hh"
#include "palette_index.hh"
#include "sprite.hh"
#include "sprite16_index.hh"
#include "sprite32_index.hh"

namespace sc::sprites {

    static constexpr std::uint64_t kAtlasMagicBytes{0x3476205441204353};

    /**
     * @struct atlas
     * @brief A contiguous collection of sprites.
     *
     * This class is designed to live within an `sc::core::mapped_view`.
     */
    struct alignas(core::kCacheAlignment) atlas final {
        [[nodiscard]] static constexpr bool validate(
                const void* ptr, std::size_t mapped_size) noexcept;

        // Delete constructors because the atlas is mapped, not instantiated.
        atlas() = delete;
        atlas(const atlas&) = delete;
        atlas(atlas&&) = delete;

        ~atlas() = default;

        atlas& operator=(const atlas&) = delete;
        atlas& operator=(atlas&&) = delete;

        [[nodiscard]] constexpr auto operator[](palette_index i) const noexcept
                -> const palette&;

        [[nodiscard]] constexpr auto operator[](sprite16_index i) const noexcept
                -> const sprite16&;

        [[nodiscard]] constexpr auto operator[](sprite32_index i) const noexcept
                -> const sprite32&;

        [[nodiscard]] constexpr auto data() const noexcept -> const std::byte*;

        [[nodiscard]] constexpr auto palette_span() const noexcept
                -> std::span<const palette>;

        [[nodiscard]] constexpr auto sprite16_span() const noexcept
                -> std::span<const sprite16>;

        [[nodiscard]] constexpr auto sprite32_span() const noexcept
                -> std::span<const sprite32>;

        struct alignas(core::kNeonAlignment) metadata final {
            const std::uint64_t magic;
            const std::uint32_t sprite16_count;
            const std::uint16_t sprite32_count;
            const std::uint16_t palette_count;
        } meta;
    };

    [[nodiscard]] constexpr bool atlas::validate(
            const void* ptr, const std::size_t mapped_size) noexcept
    {
        if (!ptr || mapped_size < sizeof(atlas))
            return false;

        const auto [magic, sprite16_count, sprite32_count, palette_count]{
                static_cast<const atlas*>(ptr)->meta};
        if (magic != kAtlasMagicBytes)
            return false;

        const std::size_t expected_size{sizeof(metadata) +
                sizeof(palette) * palette_count +
                sizeof(sprite16) * sprite16_count +
                sizeof(sprite32) * sprite32_count};
        return mapped_size >= expected_size;
    }

    [[nodiscard]] constexpr auto atlas::operator[](
            const palette_index i) const noexcept -> const palette&
    {
        return palette_span()[static_cast<std::size_t>(i)];
    }

    [[nodiscard]] constexpr auto atlas::operator[](
            const sprite16_index i) const noexcept -> const sprite16&
    {
        return sprite16_span()[static_cast<std::size_t>(i)];
    }

    [[nodiscard]] constexpr auto atlas::operator[](
            const sprite32_index i) const noexcept -> const sprite32&
    {
        return sprite32_span()[static_cast<std::size_t>(i)];
    }

    [[nodiscard]] constexpr auto atlas::data() const noexcept
            -> const std::byte*
    {
        return reinterpret_cast<const std::byte*>(&meta + 1);
    }

    [[nodiscard]] constexpr auto atlas::palette_span() const noexcept
            -> std::span<const palette>
    {
        return {reinterpret_cast<const palette*>(data()),
                sizeof(palette) * meta.palette_count};
    }

    [[nodiscard]] constexpr auto atlas::sprite16_span() const noexcept
            -> std::span<const sprite16>
    {
        return {reinterpret_cast<const sprite16*>(
                        data() + sizeof(palette) * meta.palette_count),
                sizeof(sprite16) * meta.sprite16_count};
    }

    [[nodiscard]] constexpr auto atlas::sprite32_span() const noexcept
            -> std::span<const sprite32>
    {
        return {reinterpret_cast<const sprite32*>(
                        data() + sizeof(palette) * meta.palette_count) +
                        sizeof(sprite16) * meta.sprite16_count,
                sizeof(sprite32) * meta.sprite32_count};
    }

} // namespace sc::sprites
